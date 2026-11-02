#include <android_native_app_glue.h>
#include <android/input.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "PAS", __VA_ARGS__)
namespace {
struct Point { float x,y,pressure,tilt; };
struct Stroke { std::vector<Point> points; float r=0.08f,g=0.55f,b=1.0f; float size=5.f; bool erase=false; };
struct Vertex { float x,y,r,g,b,a; };
struct App {
 android_app* state{}; EGLDisplay display=EGL_NO_DISPLAY; EGLSurface surface=EGL_NO_SURFACE; EGLContext context=EGL_NO_CONTEXT;
 GLuint program=0,vao=0,vbo=0; GLint viewportLoc=-1; int width=1,height=1;
 std::vector<Stroke> strokes,redo; Stroke active; bool drawing=false,eraser=false;
 int tool=0; float color[3]{0.08f,0.55f,1.f}; float brush=7.f; bool dirty=true;
};
GLuint shader(GLenum type,const char* src){ GLuint s=glCreateShader(type); glShaderSource(s,1,&src,nullptr); glCompileShader(s); GLint ok=0; glGetShaderiv(s,GL_COMPILE_STATUS,&ok); if(!ok){char log[512];glGetShaderInfoLog(s,512,nullptr,log);LOGE("shader: %s",log);} return s; }
bool initEgl(App& a){
 a.display=eglGetDisplay(EGL_DEFAULT_DISPLAY); if(a.display==EGL_NO_DISPLAY||!eglInitialize(a.display,nullptr,nullptr))return false;
 const EGLint cfg[]={EGL_RENDERABLE_TYPE,EGL_OPENGL_ES3_BIT,EGL_SURFACE_TYPE,EGL_WINDOW_BIT,EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_ALPHA_SIZE,8,EGL_NONE}; EGLConfig c; EGLint n;
 if(!eglChooseConfig(a.display,cfg,&c,1,&n)||!n)return false;
 const EGLint ctx[]={EGL_CONTEXT_CLIENT_VERSION,3,EGL_NONE}; a.context=eglCreateContext(a.display,c,EGL_NO_CONTEXT,ctx);
 a.surface=eglCreateWindowSurface(a.display,c,a.state->window,nullptr);
 if(a.context==EGL_NO_CONTEXT||a.surface==EGL_NO_SURFACE||!eglMakeCurrent(a.display,a.surface,a.surface,a.context))return false;
 eglQuerySurface(a.display,a.surface,EGL_WIDTH,&a.width); eglQuerySurface(a.display,a.surface,EGL_HEIGHT,&a.height);
 const char* vs="#version 300 es\nlayout(location=0) in vec2 p; layout(location=1) in vec4 c; out vec4 col; void main(){gl_Position=vec4(p,0,1);gl_PointSize=clamp(c.a*2.0,1.0,96.0);col=vec4(c.rgb,1.0);}";
 const char* fs="#version 300 es\nprecision mediump float; in vec4 col; out vec4 frag; void main(){float d=length(gl_PointCoord-vec2(0.5));float aa=max(fwidth(d),0.003);float alpha=1.0-smoothstep(0.5-aa,0.5,d);if(alpha<0.01)discard;frag=vec4(col.rgb,alpha);}";
 GLuint v=shader(GL_VERTEX_SHADER,vs),f=shader(GL_FRAGMENT_SHADER,fs); a.program=glCreateProgram();glAttachShader(a.program,v);glAttachShader(a.program,f);glLinkProgram(a.program);glDeleteShader(v);glDeleteShader(f);
 glGenVertexArrays(1,&a.vao);glBindVertexArray(a.vao);glGenBuffers(1,&a.vbo);glBindBuffer(GL_ARRAY_BUFFER,a.vbo);
 glEnableVertexAttribArray(0);glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)0);
 glEnableVertexAttribArray(1);glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(2*sizeof(float)));
 glUseProgram(a.program);glViewport(0,0,a.width,a.height);return true;
}
void shutdown(App& a){if(a.display!=EGL_NO_DISPLAY){eglMakeCurrent(a.display,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);if(a.context!=EGL_NO_CONTEXT)eglDestroyContext(a.display,a.context);if(a.surface!=EGL_NO_SURFACE)eglDestroySurface(a.display,a.surface);eglTerminate(a.display);}a.display=EGL_NO_DISPLAY;a.context=EGL_NO_CONTEXT;a.surface=EGL_NO_SURFACE;}
void pushPoint(App& a,float x,float y,float pressure,float tilt){ Point p{x,y,std::clamp(pressure,0.08f,1.5f),std::clamp(tilt,0.f,1.5708f)}; if(a.active.points.empty()||std::hypot(p.x-a.active.points.back().x,p.y-a.active.points.back().y)>0.7f)a.active.points.push_back(p); }
void finish(App& a){if(a.drawing&&!a.active.points.empty()){a.strokes.push_back(std::move(a.active));a.active=Stroke{};a.redo.clear();a.dirty=true;}a.drawing=false;}
void undo(App& a){if(!a.strokes.empty()){a.redo.push_back(std::move(a.strokes.back()));a.strokes.pop_back();a.dirty=true;}}
void redo(App& a){if(!a.redo.empty()){a.strokes.push_back(std::move(a.redo.back()));a.redo.pop_back();a.dirty=true;}}
void addStrokeVertices(const Stroke& s,std::vector<Vertex>& out,int w,int h){
 if(s.points.empty())return;
 auto emit=[&](const Point& p){float x=2.f*p.x/w-1.f,y=1.f-2.f*p.y/h;float radius=s.size*(0.35f+0.65f*p.pressure);out.push_back({x,y,s.erase?0.12f:s.r,s.erase?0.13f:s.g,s.erase?0.16f:s.b,radius});};
 if(s.points.size()==1){emit(s.points[0]);return;}
 for(size_t i=1;i<s.points.size();++i){const auto& p=s.points[i-1];const auto& q=s.points[i];float d=std::hypot(q.x-p.x,q.y-p.y);int steps=std::max(1,(int)(d/3.f));for(int k=0;k<steps;++k){float t=(float)k/steps;Point z{p.x+(q.x-p.x)*t,p.y+(q.y-p.y)*t,p.pressure+(q.pressure-p.pressure)*t,0};emit(z);}}emit(s.points.back());
}
void render(App& a){
 if(a.display==EGL_NO_DISPLAY)return;
 if(a.width<=0||a.height<=0)return;
 glViewport(0,0,a.width,a.height);glClearColor(0.055f,0.065f,0.085f,1);glClear(GL_COLOR_BUFFER_BIT);
 std::vector<Vertex> vertices;for(const auto& s:a.strokes)addStrokeVertices(s,vertices,a.width,a.height);if(a.drawing)addStrokeVertices(a.active,vertices,a.width,a.height);
 glUseProgram(a.program);glBindVertexArray(a.vao);glBindBuffer(GL_ARRAY_BUFFER,a.vbo);glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(Vertex),vertices.data(),GL_DYNAMIC_DRAW);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDrawArrays(GL_POINTS,0,(GLsizei)vertices.size());
 // Native toolbar: five large hit targets, colored indicators, and separators.
 std::vector<Vertex> ui;auto rect=[&](float x0,float y0,float x1,float y1,float r,float g,float b){for(auto p:std::vector<std::pair<float,float>>{{x0,y0},{x1,y0},{x1,y1},{x0,y0},{x1,y1},{x0,y1}})ui.push_back({2*p.first/a.width-1,1-2*p.second/a.height,r,g,b,1});};
 rect(0,0,(float)a.width,64,0.075f,0.085f,0.11f);
 rect(14,12,58,52,0.15f,0.18f,0.23f);rect(72,12,116,52,0.15f,0.18f,0.23f);rect(130,12,174,52,0.15f,0.18f,0.23f);
 rect(190,12,234,52,0.15f,0.18f,0.23f);rect(248,12,292,52,0.15f,0.18f,0.23f);
 rect(24,22,48,42,0.95f,0.95f,0.98f);rect(82,22,106,42,0.95f,0.95f,0.98f);
 rect(141,22,163,42,0.95f,0.95f,0.98f);
 rect(200,22,224,42,a.color[0],a.color[1],a.color[2]);rect(258,22,282,42,0.9f,0.25f,0.22f);
 glBufferData(GL_ARRAY_BUFFER,ui.size()*sizeof(Vertex),ui.data(),GL_DYNAMIC_DRAW);glDrawArrays(GL_TRIANGLES,0,(GLsizei)ui.size());eglSwapBuffers(a.display,a.surface);
}
int32_t input(android_app* app,AInputEvent* e){
 App& a=*static_cast<App*>(app->userData);if(AInputEvent_getType(e)!=AINPUT_EVENT_TYPE_MOTION)return 0;
 int action=AMotionEvent_getAction(e)&AMOTION_EVENT_ACTION_MASK;size_t ix=0;float x=AMotionEvent_getX(e,ix),y=AMotionEvent_getY(e,ix);
 if(action==AMOTION_EVENT_ACTION_DOWN){
   if(y<68){if(x<66)undo(a);else if(x<124)redo(a);else if(x<182){a.eraser=!a.eraser;}else if(x<240){a.color[0]=0.08f;a.color[1]=0.55f;a.color[2]=1.f;a.eraser=false;}else{a.color[0]=0.95f;a.color[1]=0.35f;a.color[2]=0.22f;a.eraser=false;}return 1;}
   a.drawing=true;a.active=Stroke{};a.active.r=a.color[0];a.active.g=a.color[1];a.active.b=a.color[2];a.active.size=a.brush;a.active.erase=a.eraser;
   float pr=AMotionEvent_getPressure(e,ix);if(pr<=0)pr=0.5f;pushPoint(a,x,y,pr,AMotionEvent_getAxisValue(e,AMOTION_EVENT_AXIS_TILT,ix));a.dirty=true;return 1;
 }
 if(action==AMOTION_EVENT_ACTION_MOVE&&a.drawing){size_t count=AMotionEvent_getPointerCount(e);for(size_t i=0;i<count;++i){float pr=AMotionEvent_getPressure(e,i);if(pr<=0)pr=0.5f;pushPoint(a,AMotionEvent_getX(e,i),AMotionEvent_getY(e,i),pr,AMotionEvent_getAxisValue(e,AMOTION_EVENT_AXIS_TILT,i));}a.dirty=true;return 1;}
 if(action==AMOTION_EVENT_ACTION_UP||action==AMOTION_EVENT_ACTION_CANCEL){finish(a);return 1;}return 0;
}
void command(android_app* app,int32_t cmd){App& a=*static_cast<App*>(app->userData);if(cmd==APP_CMD_INIT_WINDOW&&app->window){if(initEgl(a))a.dirty=true;}else if(cmd==APP_CMD_TERM_WINDOW)shutdown(a);}
}
void android_main(android_app* state){
 app_dummy();App a;a.state=state;state->userData=&a;state->onAppCmd=command;state->onInputEvent=input;
 while(!state->destroyRequested){int events;android_poll_source* source;while(ALooper_pollOnce(a.dirty?0:-1,nullptr,&events,(void**)&source)>=0){if(source)source->process(state,source);if(state->destroyRequested)break;}if(a.dirty&&a.display!=EGL_NO_DISPLAY){render(a);a.dirty=false;}}
 shutdown(a);
}
