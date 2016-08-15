#include<2DDrawer.h>
#include<geCore/ErrorPrinter.h>
#include<Font.h>

using namespace ge::gl;

std::shared_ptr<ge::gl::Texture>createFontTexture(){
  const uint32_t w=ge::res::font::width;
  const uint32_t h=ge::res::font::height;
  uint8_t bytes[w*h];
  auto result = std::make_shared<ge::gl::Texture>(GL_TEXTURE_2D,GL_R8,0,w,h);
  for(uint32_t i=0;i<w*h/8;++i)
    for(size_t k=0;k<8;++k)
      bytes[i*8+k]=255*((ge::res::font::data[i]>>k)&0x1);
  result->setData2D(bytes,GL_RED,GL_UNSIGNED_BYTE,0,0,0,w,h,w);
  result->generateMipmap();
  result->texParameteri(GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
  result->texParameteri(GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  return result;
}

class Drawable{
  public:
    enum Type{
      LINE,
      POINT,
      CIRCLE,
      TRIANGLE,
      TEXT,
      SPLINE,
    }type;
    float color[4];
    Drawable(Type type,float r=1,float g=1,float b=1,float a=1){
      assert(this!=nullptr);
      this->setColor(r,g,b,a);
      this->type = type;
    }
    void setColor(float r,float g,float b,float a){
      assert(this!=nullptr);
      this->color[0]=r;
      this->color[1]=g;
      this->color[2]=b;
      this->color[3]=a;
    }
    virtual ~Drawable(){}
};

class Line: public Drawable{
  public:
    Line(float ax,float ay,float bx,float by,float w):Drawable(LINE){
      assert(this!=nullptr);
      this->a[0]=ax;
      this->a[1]=ay;
      this->b[0]=bx;
      this->b[1]=by;
      this->width = w;
    }
    float a[2];
    float b[2];
    float width;
};

class Point: public Drawable{
  public:
    Point(float x,float y,float rd):Drawable(POINT){
      assert(this!=nullptr);
      this->coord[0]=x;
      this->coord[1]=y;
      this->size = rd;
    }
    float coord[2];
    float size;
};

class Circle: public Drawable{
  public:
    Circle(float x,float y,float rd,float width):Drawable(CIRCLE){
      assert(this!=nullptr);
      this->coord[0]=x;
      this->coord[1]=y;
      this->size = rd;
      this->width = width;
    }
    float coord[2];
    float size;
    float width;
};

class Triangle: public Drawable{
  public:
    Triangle(float ax,float ay,float bx,float by,float cx,float cy):Drawable(TRIANGLE){
      assert(this!=nullptr);
      this->a[0]=ax;
      this->a[1]=ay;
      this->b[0]=bx;
      this->b[1]=by;
      this->c[0]=cx;
      this->c[0]=cy;
    }
    float a[2];
    float b[2];
    float c[2];
};

class Text: public Drawable{
  public:
    Text(std::string const&data,float size,float x,float y,float vx,float vy):Drawable(TEXT){
      assert(this!=nullptr);
      this->data = data;
      this->size = size;
      this->coord[0] = x;
      this->coord[1] = y;
      this->dir[0] = vx;
      this->dir[1] = vy;
    }
    std::string data;
    float size;
    float coord[2];
    float dir[2];
};

class Spline: public Drawable{
  public:
    Spline(float ax,float ay,float bx,float by,float cx,float cy,float dx,float dy,float width):Drawable(SPLINE){
      assert(this!=nullptr);
      this->a[0]=ax;
      this->a[1]=ay;
      this->b[0]=bx;
      this->b[1]=by;
      this->c[0]=cx;
      this->c[1]=cy;
      this->d[0]=dx;
      this->d[1]=dy;
      this->width = width;
    }
    float a[2];
    float b[2];
    float c[2];
    float d[2];
    float width;
};

class Draw2DImpl{
  public:
    Context gl;
    struct Viewport{
      uint32_t w;
      uint32_t h;
      uint32_t x = 0;
      uint32_t y = 0;
    }viewport;
    std::map<size_t,Drawable*>primitives;
    float pixelSize = 1.f;
    float x = 0;
    float y = 0;
    Draw2DImpl(Context const&g,uint32_t w,uint32_t h):gl(g){
      assert(this!=nullptr);
      this->viewport.w = w;
      this->viewport.h = h;
    }
    ~Draw2DImpl(){
      assert(this!=nullptr);
      for(auto const&x:this->primitives)
        delete x.second;
    }
    bool convertedForDrawing = false;
    std::shared_ptr<Buffer>lineBuffer;
    std::shared_ptr<VertexArray>lineVAO;
    std::shared_ptr<Program>lineProgram;
    size_t nofLines;
    std::shared_ptr<Buffer>pointBuffer;
    std::shared_ptr<VertexArray>pointVAO;
    std::shared_ptr<Program>pointProgram;
    size_t nofPoints;
    std::shared_ptr<Buffer>circleBuffer;
    std::shared_ptr<VertexArray>circleVAO;
    std::shared_ptr<Program>circleProgram;
    size_t nofCircles;
    std::shared_ptr<Buffer>triangleBuffer;
    std::shared_ptr<VertexArray>triangleVAO;
    std::shared_ptr<Program>triangleProgram;
    size_t nofTriangles;
    std::shared_ptr<Buffer>splineBuffer;
    std::shared_ptr<VertexArray>splineVAO;
    std::shared_ptr<Program>splineProgram;
    size_t nofSplines;
    std::shared_ptr<Buffer>textBuffer;
    std::shared_ptr<VertexArray>textVAO;
    std::shared_ptr<Program>textProgram;
    std::shared_ptr<Texture>fontTexture;
    size_t nofCharacters;
};

Draw2D::Draw2D(Context const&gl,uint32_t w,uint32_t h){
  assert(this!=nullptr);
  this->_impl = new Draw2DImpl(gl,w,h);
  std::string lineVS=
    "#version 450\n"
    "layout(location=0)in vec4  color;\n"
    "layout(location=1)in vec2  a;\n"
    "layout(location=2)in vec2  b;\n"
    "layout(location=3)in float width;\n"
    "out vec2 vA;\n"
    "out vec2 vB;\n"
    "out float vWidth;\n"
    "out vec4 vColor;\n"
    "void main(){\n"
    "  vA = a;\n"
    "  vB = b;\n"
    "  vWidth = width;\n"
    "  vColor = color;\n"
    "}\n";
  std::string lineGS=
    "#version 450\n"
    "layout(points)in;\n"
    "layout(triangle_strip,max_vertices=4)out;\n"
    "uniform vec2 pos = vec2(0);\n"
    "uniform float pixelSize = 1;\n"
    "uniform uvec2 windowSize = uvec2(1024,768);\n"
    "in vec2 vA[];\n"
    "in vec2 vB[];\n"
    "in float vWidth[];\n"
    "in vec4 vColor[];\n"
    "out vec4 gColor;\n"
    "void main(){\n"
    "  vec2 v = vB[0]-vA[0];\n"
    "  vec2 s = vA[0];\n"
    "  vec2 r = normalize(vec2(-v.y,v.x));\n"
    "  float w = vWidth[0];\n"
    "  for(int i=0;i<4;++i){\n"
    "    vec2 c = (pos+s+v*(i/2)+r*(+1-2*(i%2))*w)*pixelSize/vec2(windowSize)*2;\n"
    "    gl_Position = vec4(c,1,1);\n"
    "    gColor = vColor[0];\n"
    "    EmitVertex();\n"
    "  }\n"
    "  EndPrimitive();\n"
    "}\n";
  std::string lineFS=
    "#version 450\n"
    "out vec4 fColor;\n"
    "in vec4 gColor;\n"
    "void main(){\n"
    "  fColor = gColor;\n"
    "}\n";
  this->_impl->lineProgram = std::make_shared<Program>();
  this->_impl->lineProgram->link(
      {std::make_shared<Shader>(GL_VERTEX_SHADER,lineVS),
      std::make_shared<Shader>(GL_GEOMETRY_SHADER,lineGS),
      std::make_shared<Shader>(GL_FRAGMENT_SHADER,lineFS)});

  std::string pointVS=
    "#version 450\n"
    "layout(location=0)in vec4  color;\n"
    "layout(location=1)in vec2  coord;\n"
    "layout(location=2)in float size;\n"
    "out vec2 vCoord;\n"
    "out float vSize;\n"
    "out vec4 vColor;\n"
    "void main(){\n"
    "  vCoord = coord;\n"
    "  vSize = size;\n"
    "  vColor = color;\n"
    "}\n";
  std::string pointGS=
    "#version 450\n"
    "layout(points)in;\n"
    "layout(triangle_strip,max_vertices=4)out;\n"
    "uniform vec2 pos = vec2(0);\n"
    "uniform float pixelSize = 1;\n"
    "uniform uvec2 windowSize = uvec2(1024,768);\n"
    "in vec2 vCoord[];\n"
    "in float vSize[];\n"
    "in vec4 vColor[];\n"
    "out vec4 gColor;\n"
    "out vec2 gCoord;\n"
    "void main(){\n"
    "  vec2 s = vCoord[0];\n"
    "  float w = vSize[0];\n"
    "  for(int i=0;i<4;++i){\n"
    "    vec2 c = (pos+s+(vec2(i/2,i%2)*2-1)*w)*pixelSize/vec2(windowSize)*2;\n"
    "    gl_Position = vec4(c,1,1);\n"
    "    gColor = vColor[0];\n"
    "    gCoord = vec2(i/2,i%2);\n"
    "    EmitVertex();\n"
    "  }\n"
    "  EndPrimitive();\n"
    "}\n";
  std::string pointFS=
    "#version 450\n"
    "out vec4 fColor;\n"
    "in vec4 gColor;\n"
    "in vec2 gCoord;\n"
    "void main(){\n"
    "  if(length(gCoord-vec2(0.5))>0.5)discard;\n"
    "  fColor = gColor;\n"
    "}\n";
  this->_impl->pointProgram = std::make_shared<Program>();
  this->_impl->pointProgram->link(
      {std::make_shared<Shader>(GL_VERTEX_SHADER,pointVS),
      std::make_shared<Shader>(GL_GEOMETRY_SHADER,pointGS),
      std::make_shared<Shader>(GL_FRAGMENT_SHADER,pointFS)});

  std::string circleVS=
    "#version 450\n"
    "layout(location=0)in vec4  color;\n"
    "layout(location=1)in vec2  coord;\n"
    "layout(location=2)in float size;\n"
    "layout(location=3)in float width;\n"
    "out vec4 vColor;\n"
    "out vec2 vCoord;\n"
    "out float vSize;\n"
    "out float vWidth;\n"
    "void main(){\n"
    "  vColor = color;\n"
    "  vCoord = coord;\n"
    "  vSize = size;\n"
    "  vWidth = width;\n"
    "}\n";
  std::string circleGS=
    "#version 450\n"
    "layout(points)in;\n"
    "layout(triangle_strip,max_vertices=4)out;\n"
    "uniform vec2 pos = vec2(0);\n"
    "uniform float pixelSize = 1;\n"
    "uniform uvec2 windowSize = uvec2(1024,768);\n"
    "in vec4 vColor[];\n"
    "in vec2 vCoord[];\n"
    "in float vSize[];\n"
    "in float vWidth[];\n"
    "out vec4 gColor;\n"
    "out vec2 gCoord;\n"
    "out float gMin;\n"
    "void main(){\n"
    "  vec2 s = vCoord[0];\n"
    "  float w = vSize[0];\n"
    "  for(int i=0;i<4;++i){\n"
    "    vec2 c = (pos+s+(vec2(i/2,i%2)*2-1)*w)*pixelSize/vec2(windowSize)*2;\n"
    "    gl_Position = vec4(c,1,1);\n"
    "    gColor = vColor[0];\n"
    "    gCoord = vec2(i/2,i%2);\n"
    "    gMin = vWidth[0]/w;\n"
    "    EmitVertex();\n"
    "  }\n"
    "  EndPrimitive();\n"
    "}\n";
  std::string circleFS=
    "#version 450\n"
    "out vec4 fColor;\n"
    "in vec4 gColor;\n"
    "in vec2 gCoord;\n"
    "in float gMin;\n"
    "void main(){\n"
    "  float dist = length(gCoord-vec2(0.5));\n"
    "  if(dist>0.5)discard;\n"
    "  if(dist<0.5-gMin/2)discard;\n"
    "  fColor = gColor;\n"
    "}\n";
  this->_impl->circleProgram = std::make_shared<Program>();
  this->_impl->circleProgram->link(
      {std::make_shared<Shader>(GL_VERTEX_SHADER,circleVS),
      std::make_shared<Shader>(GL_GEOMETRY_SHADER,circleGS),
      std::make_shared<Shader>(GL_FRAGMENT_SHADER,circleFS)});

  std::string triangleVS=
    "#version 450\n"
    "layout(location=0)in vec4  color;\n"
    "layout(location=1)in vec2  coord;\n"
    "uniform vec2 pos = vec2(0);\n"
    "uniform float pixelSize = 1;\n"
    "uniform uvec2 windowSize = uvec2(1024,768);\n"
    "out vec4 vColor;\n"
    "void main(){\n"
    "  vec2 c = (pos+coord)*pixelSize/vec2(windowSize)*2;\n"
    "  gl_Position = vec4(c,1,1);\n"
    "  vColor = color;\n"
    "}\n";
  std::string triangleFS=
    "#version 450\n"
    "out vec4 fColor;\n"
    "in vec4 vColor;\n"
    "void main(){\n"
    "  fColor = vColor;\n"
    "}\n";
  this->_impl->triangleProgram = std::make_shared<Program>();
  this->_impl->triangleProgram->link(
      {std::make_shared<Shader>(GL_VERTEX_SHADER,triangleVS),
      std::make_shared<Shader>(GL_FRAGMENT_SHADER,triangleFS)});

  std::string splineVS=
    "#version 450\n"
    "layout(location=0)in vec4  color;\n"
    "layout(location=1)in vec2  a;\n"
    "layout(location=2)in vec2  b;\n"
    "layout(location=3)in vec2  c;\n"
    "layout(location=4)in vec2  d;\n"
    "layout(location=5)in float width;\n"
    "out vec2 vA;\n"
    "out vec2 vB;\n"
    "out vec2 vC;\n"
    "out vec2 vD;\n"
    "out float vWidth;\n"
    "out vec4 vColor;\n"
    "void main(){\n"
    "  vA = a;\n"
    "  vB = b;\n"
    "  vC = c;\n"
    "  vD = d;\n"
    "  vWidth = width;\n"
    "  vColor = color;\n"
    "}\n";
  std::string splineCS=
    "#version 450\n"
    "layout(vertices=1)out;\n"
    "in vec2 vA[];\n"
    "in vec2 vB[];\n"
    "in vec2 vC[];\n"
    "in vec2 vD[];\n"
    "in float vWidth[];\n"
    "in vec4 vColor[];\n"
    "patch out vec2 cPos[4];\n"
    "patch out vec4 cColor;\n"
    "patch out float cWidth;\n"
    "void main(){\n"
    "  cColor = vColor[0];\n"
    "  cWidth = vWidth[0];\n"
    "  cPos[0] = vA[0];\n"
    "  cPos[1] = vB[0];\n"
    "  cPos[2] = vC[0];\n"
    "  cPos[3] = vD[0];\n"
    "  gl_TessLevelInner[1]=64;\n"
    "  gl_TessLevelInner[0]=1;\n"
    "  gl_TessLevelOuter[1]=64;\n"
    "  gl_TessLevelOuter[0]=1;\n"
    "  gl_TessLevelOuter[2]=1;\n"
    "  gl_TessLevelOuter[3]=1;\n"
  "}\n";
  std::string splineES=
    "#version 450\n"
    "layout(isolines,equal_spacing)in;\n"
    "patch in vec2 cPos[4];\n"
    "patch in vec4 cColor;\n"
    "patch in float cWidth;\n"
    "out vec2 eA;\n"
    "out float eWidth;\n"
    "out vec4 eColor;\n"
    "float cattmulrom(float a,float b,float c,float d,float t1){\n"
    "  float t2=t1*t1;\n"
    "  float t3=t2*t1;\n"
    "  return\n"
    "    ( -.5*t3 +    t2 - .5*t1 + 0 )*a+\n"
    "    ( 1.5*t3 -2.5*t2 + 0     + 1.)*b+\n"
    "    (-1.5*t3 +  2*t2 + .5*t1 + 0.)*c+\n"
    "    (  .5*t3 - .5*t2 + 0     + 0.)*d;\n"
    "}\n"
    "void main(){\n"
    "  float xx=cattmulrom(cPos[0].x,cPos[1].x,cPos[2].x,cPos[3].x,gl_TessCoord.x);\n"
    "  float yy=cattmulrom(cPos[0].y,cPos[1].y,cPos[2].y,cPos[3].y,gl_TessCoord.x);\n"
    "  eA = vec2(xx,yy);\n"
    "  eWidth = cWidth;\n"
    "  eColor = cColor;\n"
    "}\n";

  std::string splineGS=
    "#version 450\n"
    "layout(lines)in;\n"
    "layout(triangle_strip,max_vertices=4)out;\n"
    "uniform vec2 pos = vec2(0);\n"
    "uniform float pixelSize = 1;\n"
    "uniform uvec2 windowSize = uvec2(1024,768);\n"
    "in vec2 eA[];\n"
    "in float eWidth[];\n"
    "in vec4 eColor[];\n"
    "out vec4 gColor;\n"
    "void main(){\n"
    "  vec2 v = eA[1]-eA[0];\n"
    "  vec2 s = eA[0];\n"
    "  vec2 r = normalize(vec2(-v.y,v.x));\n"
    "  float w = eWidth[0];\n"
    "  for(int i=0;i<4;++i){\n"
    "    vec2 c = (pos+s+v*(i/2)+r*(+1-2*(i%2))*w)*pixelSize/vec2(windowSize)*2;\n"
    "    gl_Position = vec4(c,1,1);\n"
    "    gColor = eColor[0];\n"
    "    EmitVertex();\n"
    "  }\n"
    "  EndPrimitive();\n"
    "}\n";
  std::string splineFS=
    "#version 450\n"
    "out vec4 fColor;\n"
    "in vec4 gColor;\n"
    "void main(){\n"
    "  fColor = gColor;\n"
    "}\n";
  this->_impl->splineProgram = std::make_shared<Program>();
  this->_impl->splineProgram->link(
      {std::make_shared<Shader>(GL_VERTEX_SHADER,splineVS),
      std::make_shared<Shader>(GL_TESS_CONTROL_SHADER,splineCS),
      std::make_shared<Shader>(GL_TESS_EVALUATION_SHADER,splineES),
      std::make_shared<Shader>(GL_GEOMETRY_SHADER,splineGS),
      std::make_shared<Shader>(GL_FRAGMENT_SHADER,splineFS)});

  this->_impl->fontTexture = createFontTexture();
  std::string textVS=
    "#version 450\n"
    "layout(location=0)in vec4  color ;\n"
    "layout(location=1)in vec2  pos   ;\n"
    "layout(location=2)in vec2  dir   ;\n"
    "layout(location=3)in float number;\n"
    "layout(location=4)in float char  ;\n"
    "layout(location=5)in float size  ;\n"
    "out vec4  vColor ;\n"
    "out vec2  vPos   ;\n"
    "out vec2  vDir   ;\n"
    "out float vNumber;\n"
    "out float vChar  ;\n"
    "out float vSize  ;\n"
    "void main(){\n"
    "  vColor  = color ;\n"
    "  vPos    = pos   ;\n"
    "  vDir    = dir   ;\n"
    "  vNumber = number;\n"
    "  vChar   = char  ;\n"
    "  vSize   = size  ;\n"
    "}\n";
  std::string textGS=
    "#version 450\n"
    "layout(points)in;\n"
    "layout(triangle_strip,max_vertices=4)out;\n"
    "uniform vec2 pos = vec2(0);\n"
    "uniform float pixelSize = 1;\n"
    "uniform uvec2 windowSize = uvec2(1024,768);\n"
    "in vec4  vColor [];\n"
    "in vec2  vPos   [];\n"
    "in vec2  vDir   [];\n"
    "in float vNumber[];\n"
    "in float vChar  [];\n"
    "in float vSize  [];\n"
    "out vec4  gColor;\n"
    "out float gChar ;\n"
    "out vec2  gCoord;\n"
    "void main(){\n"
    "  vec2 v = normalize(vDir[0]);\n"
    "  vec2 s = vPos[0];\n"
    "  vec2 r = normalize(vec2(-v.y,v.x));\n"
    "  float w = vSize[0];\n"
    "  for(int i=0;i<4;++i){\n"
    "    vec2 c = (pos+s+v*(vNumber[0]+i%2)*w+r*(i/2)*w*2)*pixelSize/vec2(windowSize)*2;\n"
    "    gl_Position = vec4(c,1,1);\n"
    "    gColor = vColor[0];\n"
    "    gChar = vChar[0];\n"
    "    gCoord = vec2(i%2,i/2);\n"
    "    EmitVertex();\n"
    "  }\n"
    "  EndPrimitive();\n"
    "}\n";
  std::string textFS=
    "#version 450\n"
    "out vec4 fColor;\n"
    "in vec4 gColor;\n"
    "in float gChar;\n"
    "in vec2 gCoord;\n"
    "layout(binding=0)uniform sampler2D fontTexture;\n"
    "const uint fontWidth = 1330;\n"
    "const uint fontHeight = 28;\n"
    "const uint nofCharacters = 95;\n"
    "const uint characterWidth = fontWidth/nofCharacters;\n"
    "const uint characterHeight = fontHeight;\n"
    "const float characterWidthNormalized = 1.f/nofCharacters;\n"
    "#define CH_space 32\n"
    "float font(int ch,vec2 coord){\n"
    "  int id = ch-CH_space;\n"
    "  return texture(fontTexture,vec2((id+coord.x)*characterWidthNormalized,coord.y)).r;\n"
    "}\n"
    "void main(){\n"
    "  fColor = gColor*font(int(gChar),gCoord);\n"
    "}\n";
  this->_impl->textProgram = std::make_shared<Program>();
  this->_impl->textProgram->link(
      {std::make_shared<Shader>(GL_VERTEX_SHADER,textVS),
      std::make_shared<Shader>(GL_GEOMETRY_SHADER,textGS),
      std::make_shared<Shader>(GL_FRAGMENT_SHADER,textFS)});
}

Draw2D::~Draw2D(){
  assert(this!=nullptr);
  delete this->_impl;
}

void Draw2D::setViewportSize(uint32_t x,uint32_t y,uint32_t w,uint32_t h){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  this->_impl->viewport.x = x;
  this->_impl->viewport.y = y;
  this->_impl->viewport.w = w;
  this->_impl->viewport.h = h;
}

void Draw2D::draw(){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto const&gl=this->_impl->gl;
  auto const&viewport = this->_impl->viewport;
  gl.glViewport(viewport.x,viewport.y,viewport.w,viewport.h);
  gl.glClearColor(0,0,0,0);
  gl.glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
  if(!this->_impl->convertedForDrawing){
    std::vector<float>lineData;
    for(auto const&x:this->_impl->primitives){
      if(x.second->type == Drawable::LINE){
        auto l = (Line*)x.second;
        lineData.push_back(l->color[0]);
        lineData.push_back(l->color[1]);
        lineData.push_back(l->color[2]);
        lineData.push_back(l->color[3]);
        lineData.push_back(l->a[0]);
        lineData.push_back(l->a[1]);
        lineData.push_back(l->b[0]);
        lineData.push_back(l->b[1]);
        lineData.push_back(l->width);
      }
    }
    this->_impl->lineBuffer = std::make_shared<Buffer>(lineData.size()*sizeof(float),lineData.data());
    this->_impl->nofLines = lineData.size()/9;
    this->_impl->lineVAO = std::make_shared<VertexArray>();
    this->_impl->lineVAO->addAttrib(this->_impl->lineBuffer,
        0,4,GL_FLOAT,sizeof(float)*9,sizeof(float)*0);
    this->_impl->lineVAO->addAttrib(this->_impl->lineBuffer,
        1,2,GL_FLOAT,sizeof(float)*9,sizeof(float)*4);
    this->_impl->lineVAO->addAttrib(this->_impl->lineBuffer,
        2,2,GL_FLOAT,sizeof(float)*9,sizeof(float)*6);
    this->_impl->lineVAO->addAttrib(this->_impl->lineBuffer,
        3,1,GL_FLOAT,sizeof(float)*9,sizeof(float)*8);

    std::vector<float>pointData;
    for(auto const&x:this->_impl->primitives){
      if(x.second->type == Drawable::POINT){
        auto l = (Point*)x.second;
        pointData.push_back(l->color[0]);
        pointData.push_back(l->color[1]);
        pointData.push_back(l->color[2]);
        pointData.push_back(l->color[3]);
        pointData.push_back(l->coord[0]);
        pointData.push_back(l->coord[1]);
        pointData.push_back(l->size);
      }
    }
    this->_impl->pointBuffer = std::make_shared<Buffer>(pointData.size()*sizeof(float),pointData.data());
    this->_impl->nofPoints = pointData.size()/7;
    this->_impl->pointVAO = std::make_shared<VertexArray>();
    this->_impl->pointVAO->addAttrib(this->_impl->pointBuffer,
        0,4,GL_FLOAT,sizeof(float)*7,sizeof(float)*0);
    this->_impl->pointVAO->addAttrib(this->_impl->pointBuffer,
        1,2,GL_FLOAT,sizeof(float)*7,sizeof(float)*4);
    this->_impl->pointVAO->addAttrib(this->_impl->pointBuffer,
        2,1,GL_FLOAT,sizeof(float)*7,sizeof(float)*6);

    std::vector<float>circleData;
    for(auto const&x:this->_impl->primitives){
      if(x.second->type == Drawable::CIRCLE){
        auto l = (Circle*)x.second;
        circleData.push_back(l->color[0]);
        circleData.push_back(l->color[1]);
        circleData.push_back(l->color[2]);
        circleData.push_back(l->color[3]);
        circleData.push_back(l->coord[0]);
        circleData.push_back(l->coord[1]);
        circleData.push_back(l->size);
        circleData.push_back(l->width);
      }
    }
    this->_impl->circleBuffer = std::make_shared<Buffer>(circleData.size()*sizeof(float),circleData.data());
    this->_impl->nofCircles = circleData.size()/8;
    this->_impl->circleVAO = std::make_shared<VertexArray>();
    this->_impl->circleVAO->addAttrib(this->_impl->circleBuffer,
        0,4,GL_FLOAT,sizeof(float)*8,sizeof(float)*0);
    this->_impl->circleVAO->addAttrib(this->_impl->circleBuffer,
        1,2,GL_FLOAT,sizeof(float)*8,sizeof(float)*4);
    this->_impl->circleVAO->addAttrib(this->_impl->circleBuffer,
        2,1,GL_FLOAT,sizeof(float)*8,sizeof(float)*6);
    this->_impl->circleVAO->addAttrib(this->_impl->circleBuffer,
        3,1,GL_FLOAT,sizeof(float)*8,sizeof(float)*7);


    std::vector<float>triangleData;
    for(auto const&x:this->_impl->primitives){
      if(x.second->type == Drawable::TRIANGLE){
        auto l = (Triangle*)x.second;
        for(size_t i=0;i<3;++i){
          triangleData.push_back(l->color[0]);
          triangleData.push_back(l->color[1]);
          triangleData.push_back(l->color[2]);
          triangleData.push_back(l->color[3]);
          if(i==0){
            triangleData.push_back(l->a[0]);
            triangleData.push_back(l->a[1]);
          }
          if(i==1){
            triangleData.push_back(l->b[0]);
            triangleData.push_back(l->b[1]);
          }
          if(i==2){
            triangleData.push_back(l->c[0]);
            triangleData.push_back(l->c[1]);
          }
        }
      }
    }
    this->_impl->triangleBuffer = std::make_shared<Buffer>(triangleData.size()*sizeof(float),triangleData.data());
    this->_impl->nofTriangles = triangleData.size()/6/3;
    this->_impl->triangleVAO = std::make_shared<VertexArray>();
    this->_impl->triangleVAO->addAttrib(this->_impl->triangleBuffer,
        0,4,GL_FLOAT,sizeof(float)*6,sizeof(float)*0);
    this->_impl->triangleVAO->addAttrib(this->_impl->triangleBuffer,
        1,2,GL_FLOAT,sizeof(float)*6,sizeof(float)*4);


    std::vector<float>splineData;
    for(auto const&x:this->_impl->primitives){
      if(x.second->type == Drawable::SPLINE){
        auto l = (Spline*)x.second;
        splineData.push_back(l->color[0]);
        splineData.push_back(l->color[1]);
        splineData.push_back(l->color[2]);
        splineData.push_back(l->color[3]);
        splineData.push_back(l->a[0]);
        splineData.push_back(l->a[1]);
        splineData.push_back(l->b[0]);
        splineData.push_back(l->b[1]);
        splineData.push_back(l->c[0]);
        splineData.push_back(l->c[1]);
        splineData.push_back(l->d[0]);
        splineData.push_back(l->d[1]);
        splineData.push_back(l->width);
      }
    }
    this->_impl->splineBuffer = std::make_shared<Buffer>(splineData.size()*sizeof(float),splineData.data());
    this->_impl->nofSplines = splineData.size()/13;
    this->_impl->splineVAO = std::make_shared<VertexArray>();
    this->_impl->splineVAO->addAttrib(this->_impl->splineBuffer,
        0,4,GL_FLOAT,sizeof(float)*13,sizeof(float)*0 );
    this->_impl->splineVAO->addAttrib(this->_impl->splineBuffer,
        1,2,GL_FLOAT,sizeof(float)*13,sizeof(float)*4 );
    this->_impl->splineVAO->addAttrib(this->_impl->splineBuffer,
        2,2,GL_FLOAT,sizeof(float)*13,sizeof(float)*6 );
    this->_impl->splineVAO->addAttrib(this->_impl->splineBuffer,
        3,2,GL_FLOAT,sizeof(float)*13,sizeof(float)*8 );
    this->_impl->splineVAO->addAttrib(this->_impl->splineBuffer,
        4,2,GL_FLOAT,sizeof(float)*13,sizeof(float)*10);
    this->_impl->splineVAO->addAttrib(this->_impl->splineBuffer,
        5,1,GL_FLOAT,sizeof(float)*13,sizeof(float)*12);

    std::vector<float>textData;
    for(auto const&x:this->_impl->primitives){
      if(x.second->type == Drawable::TEXT){
        auto l = (Text*)x.second;
        int32_t c=0;
        for(auto const&x:l->data){
          textData.push_back(l->color[0]);
          textData.push_back(l->color[1]);
          textData.push_back(l->color[2]);
          textData.push_back(l->color[3]);
          textData.push_back(l->coord[0]);
          textData.push_back(l->coord[1]);
          textData.push_back(l->dir[0]);
          textData.push_back(l->dir[1]);
          textData.push_back(c++);
          textData.push_back(x);
          textData.push_back(l->size);
        }
      }
    }
    this->_impl->textBuffer = std::make_shared<Buffer>(textData.size()*sizeof(float),textData.data());
    this->_impl->nofCharacters = textData.size()/11;
    this->_impl->textVAO = std::make_shared<VertexArray>();
    this->_impl->textVAO->addAttrib(this->_impl->textBuffer,
        0,4,GL_FLOAT,sizeof(float)*11,sizeof(float)*0);
    this->_impl->textVAO->addAttrib(this->_impl->textBuffer,
        1,2,GL_FLOAT,sizeof(float)*11,sizeof(float)*4);
    this->_impl->textVAO->addAttrib(this->_impl->textBuffer,
        2,2,GL_FLOAT,sizeof(float)*11,sizeof(float)*6);
    this->_impl->textVAO->addAttrib(this->_impl->textBuffer,
        3,1,GL_FLOAT,sizeof(float)*11,sizeof(float)*8);
    this->_impl->textVAO->addAttrib(this->_impl->textBuffer,
        4,1,GL_FLOAT,sizeof(float)*11,sizeof(float)*9);
    this->_impl->textVAO->addAttrib(this->_impl->textBuffer,
        5,1,GL_FLOAT,sizeof(float)*11,sizeof(float)*10);

    this->_impl->convertedForDrawing = true;
  }
  this->_impl->lineProgram->use();
  this->_impl->lineProgram->set1f("pixelSize",this->_impl->pixelSize);
  this->_impl->lineProgram->set2f("pos",this->_impl->x,this->_impl->y);
  this->_impl->lineProgram->set2ui("windowSize",viewport.w,viewport.h);
  this->_impl->lineVAO->bind();
  this->_impl->gl.glDrawArrays(GL_POINTS,0,this->_impl->nofLines);
  this->_impl->lineVAO->unbind();

  this->_impl->pointProgram->use();
  this->_impl->pointProgram->set1f("pixelSize",this->_impl->pixelSize);
  this->_impl->pointProgram->set2f("pos",this->_impl->x,this->_impl->y);
  this->_impl->pointProgram->set2ui("windowSize",viewport.w,viewport.h);
  this->_impl->pointVAO->bind();
  this->_impl->gl.glDrawArrays(GL_POINTS,0,this->_impl->nofPoints);
  this->_impl->pointVAO->unbind();

  this->_impl->circleProgram->use();
  this->_impl->circleProgram->set1f("pixelSize",this->_impl->pixelSize);
  this->_impl->circleProgram->set2f("pos",this->_impl->x,this->_impl->y);
  this->_impl->circleProgram->set2ui("windowSize",viewport.w,viewport.h);
  this->_impl->circleVAO->bind();
  this->_impl->gl.glDrawArrays(GL_POINTS,0,this->_impl->nofCircles);
  this->_impl->circleVAO->unbind();

  this->_impl->triangleProgram->use();
  this->_impl->triangleProgram->set1f("pixelSize",this->_impl->pixelSize);
  this->_impl->triangleProgram->set2f("pos",this->_impl->x,this->_impl->y);
  this->_impl->triangleProgram->set2ui("windowSize",viewport.w,viewport.h);
  this->_impl->triangleVAO->bind();
  this->_impl->gl.glDrawArrays(GL_TRIANGLES,0,this->_impl->nofTriangles*3);
  this->_impl->triangleVAO->unbind();

  this->_impl->splineProgram->use();
  this->_impl->splineProgram->set1f("pixelSize",this->_impl->pixelSize);
  this->_impl->splineProgram->set2f("pos",this->_impl->x,this->_impl->y);
  this->_impl->splineProgram->set2ui("windowSize",viewport.w,viewport.h);
  this->_impl->splineVAO->bind();
  this->_impl->gl.glPatchParameteri(GL_PATCH_VERTICES,1);
  this->_impl->gl.glDrawArrays(GL_PATCHES,0,this->_impl->nofSplines);
  this->_impl->splineVAO->unbind();

  this->_impl->textProgram->use();
  this->_impl->textProgram->set1f("pixelSize",this->_impl->pixelSize);
  this->_impl->textProgram->set2f("pos",this->_impl->x,this->_impl->y);
  this->_impl->textProgram->set2ui("windowSize",viewport.w,viewport.h);
  this->_impl->textVAO->bind();
  this->_impl->fontTexture->bind(0);
  this->_impl->gl.glDrawArrays(GL_POINTS,0,this->_impl->nofCharacters);
  this->_impl->textVAO->unbind();
}

void Draw2D::setScale(float pixelSize){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  this->_impl->pixelSize = pixelSize;
}

void Draw2D::setPosition(float x,float y){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  this->_impl->x = x;
  this->_impl->y = y;
}

size_t Draw2D::addLine(float ax,float ay,float bx,float by,float w,float r,float g,float b,float a){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  size_t id=this->_impl->primitives.size();
  this->_impl->primitives[id] = new Line(ax,ay,bx,by,w);
  this->setColor(id,r,g,b,a);
  return id;
}

size_t Draw2D::addPoint(float x,float y,float rd,float r,float g,float b,float a){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  size_t id=this->_impl->primitives.size();
  this->_impl->primitives[id] = new Point(x,y,rd);
  this->setColor(id,r,g,b,a);
  return id;
}

size_t Draw2D::addCircle(float x,float y,float rd,float w,float r,float g,float b,float a){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  size_t id=this->_impl->primitives.size();
  this->_impl->primitives[id] = new Circle(x,y,rd,w);
  this->setColor(id,r,g,b,a);
  return id;
}

size_t Draw2D::addTriangle(float ax,float ay,float bx,float by,float cx,float cy,float r,float g,float b,float a){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  size_t id=this->_impl->primitives.size();
  this->_impl->primitives[id] = new Triangle(ax,ay,bx,by,cx,cy);
  this->setColor(id,r,g,b,a);
  return id;
}

size_t Draw2D::addText(std::string const&data,float size,float x,float y,float vx,float vy,float r,float g,float b,float a){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  size_t id=this->_impl->primitives.size();
  this->_impl->primitives[id] = new Text(data,size,x,y,vx,vy);
  this->setColor(id,r,g,b,a);
  return id;
}

size_t Draw2D::addSpline(float ax,float ay,float bx,float by,float cx,float cy,float dx,float dy,float width,float r,float g,float b,float a){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  size_t id=this->_impl->primitives.size();
  this->_impl->primitives[id] = new Spline(ax,ay,bx,by,cx,cy,dx,dy,width);
  this->setColor(id,r,g,b,a);
  return id;
}


void Draw2D::setColor(size_t id,float r,float g,float b,float a){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto ii=this->_impl->primitives.find(id);
  if(ii==this->_impl->primitives.end()){
    ge::core::printError(GE_CORE_FCENAME,"no such primitive",id,r,g,b,a);
    return;
  }
  ii->second->setColor(r,g,b,a);
  this->_impl->convertedForDrawing = false;
}

void Draw2D::erase(size_t id){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto ii=this->_impl->primitives.find(id);
  if(ii==this->_impl->primitives.end())return;
  delete ii->second;
  this->_impl->primitives.erase(id);
  this->_impl->convertedForDrawing = false;
}

void Draw2D::clear(){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  for(auto const&x:this->_impl->primitives){
    delete x.second;
  }
  this->_impl->primitives.clear();
}
