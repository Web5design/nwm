
#include "handler.h"

static const char broken[] = "broken";

Bool gettextprop(Display* dpy, Window w, Atom atom, char *text, unsigned int size) {
  char **list = NULL;
  int n;
  XTextProperty name;

  if(!text || size == 0)
    return False;
  text[0] = '\0';
  XGetTextProperty(dpy, w, &name, atom);
  if(!name.nitems)
    return False;
  if(name.encoding == XA_STRING)
    strncpy(text, (char *)name.value, size - 1);
  else {
    if(XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success && n > 0 && *list) {
      strncpy(text, *list, size - 1);
      XFreeStringList(list);
    }
  }
  text[size - 1] = '\0';
  XFree(name.value);
  return True;
}



class Client {
public:
  int mon_id;
  Client(Window win, int mon_id, int x, int y, int width, int height, Bool isfloating) {
    fprintf( stderr, "Create client %li on monitor %d (x %d, y %d, w %d, h %d, float %d)\n", win, mon_id, x, y, width, height, isfloating);
    this->win = win;
    this->mon_id = mon_id;
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->isfloating = isfloating;
  }
  static Client* getByWindow(std::vector <Monitor>* monits, Window win);
  inline Window getWin() { return this->win; }
  void updatetitle(Display* dpy) {
    Atom NetWMName = XInternAtom(dpy, "_NET_WM_NAME", False);
    if(!gettextprop(dpy, this->win, NetWMName, this->name, sizeof this->name))
      gettextprop(dpy, this->win, XA_WM_NAME, this->name, sizeof this->name);
    if(this->name[0] == '\0') /* hack to mark broken clients */
      strcpy(this->name, broken);
  }
  void updateclass(Display* dpy) {
    XClassHint ch = { 0 };
    if(XGetClassHint(dpy, this->win, &ch)) {
      if(ch.res_class) {
        strncpy(this->klass, ch.res_class, 256-1 );
      } else {
        strncpy(this->klass, broken, 256-1 );
      }
      this->klass[256-1] = 0;
      if(ch.res_name) {
        strncpy(this->instance, ch.res_name, 256-1 );
      } else {
        strncpy(this->instance, broken, 256-1 );
      }
      this->instance[256-1] = 0;
      if(ch.res_class)
        XFree(ch.res_class);
      if(ch.res_name)
        XFree(ch.res_name);
    }
  }
  Local<Object> toNode() {
    // window object to return
    Local<Object> result = Object::New();

    // read and set the window geometry
    result->Set(String::NewSymbol("id"), Integer::New(this->win));
    result->Set(String::NewSymbol("x"), Integer::New(this->x));
    result->Set(String::NewSymbol("y"), Integer::New(this->y));
    result->Set(String::NewSymbol("monitor"), Integer::New(this->mon_id));
    result->Set(String::NewSymbol("height"), Integer::New(this->height));
    result->Set(String::NewSymbol("width"), Integer::New(this->width));
    result->Set(String::NewSymbol("title"), String::New(this->name));
    result->Set(String::NewSymbol("instance"), String::New(this->instance));
    result->Set(String::NewSymbol("class"), String::New(this->klass));
    result->Set(String::NewSymbol("isfloating"), Integer::New(this->isfloating));
    return result;
  }

  private:
    char name[256];
    char klass[256];
    char instance[256];
    int x, y, width, height;
    Client *next;
    Client *snext;
    Window win;
    Bool isfloating;
};

