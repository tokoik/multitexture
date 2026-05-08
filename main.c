#if defined(__APPLE__) || defined(MACOSX)
#  define GL_SILENCE_DEPRECATION
#  include <GLUT/glut.h>
#  include <OpenGL/glext.h>
#else
#  if defined(_WIN32)
//#    pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#    define _USE_MATH_DEFINES
#    define _CRT_SECURE_NO_WARNINGS
#  endif
#  include <GL/glut.h>
#  include <GL/glext.h>
#  if defined(_WIN32)

/*
** GL_ARB_multitexture 用の関数ポインタ
*/
PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB;

/*
** GL_ARB_multitexture 用の関数ポインタを取り出す
*/
int initMultiTexture() {
  glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
  if (!glActiveTextureARB) return 0;

  glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress("glMultiTexCoord2fARB");
  if (!glMultiTexCoord2fARB) return 0;

  return 1;
}

#  else
#    define glActiveTextureARB glActiveTexture
#    define glMultiTexCoord2fARB glMultiTexCoord2f
#  endif
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define TEXWIDTH 256
#define TEXHEIGHT 256

static char texFile0[] = "texture0.raw";
static char texFile1[] = "texture1.raw";

static GLuint texName0, texName1;

void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity();
  gluLookAt(2.0, 3.0, 4.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

  /* テクスチャ０をアクティブにする */
  glActiveTextureARB(GL_TEXTURE0_ARB);
  glBindTexture(GL_TEXTURE_2D, texName0);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glEnable(GL_TEXTURE_2D);

  /* テクスチャ１をアクティブにする */
  glActiveTextureARB(GL_TEXTURE1_ARB);
  glBindTexture(GL_TEXTURE_2D, texName1);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glEnable(GL_TEXTURE_2D);

  /* オブジェクトの描画 */
  glNormal3d(0.0, 0.0, 1.0);
  glBegin(GL_QUADS);
  glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0, 1.0);
  glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0, 1.0);
  glVertex3d(-1.0, -1.0, 0.0);
  glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0, 1.0);
  glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0, 1.0);
  glVertex3d(1.0, -1.0, 0.0);
  glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0, 0.0);
  glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0, 0.0);
  glVertex3d(1.0, 1.0, 0.0);
  glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0, 0.0);
  glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0, 0.0);
  glVertex3d(-1.0, 1.0, 0.0);
  glEnd();

  /* テクスチャ１を結合解除する */
  glDisable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

  /* テクスチャ０を結合解除する */
  glActiveTextureARB(GL_TEXTURE0_ARB);
  glDisable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

  glFlush();
}

void resize(int w, int h) {
  /* ウィンドウ全体をビューポートにする */
  glViewport(0, 0, w, h);

  /* 透視変換行列の指定 */
  glMatrixMode(GL_PROJECTION);

  /* 透視変換行列の初期化 */
  glLoadIdentity();
  gluPerspective(30.0, (double)w / (double)h, 1.0, 100.0);

  /* モデルビュー変換行列の指定 */
  glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
  switch (key) {
  case '\33':
  case 'q':
  case 'Q':
    exit(0);
  default:
    break;
  }
}

void init() {
  FILE* fp;
  static unsigned char texImage[TEXWIDTH * TEXHEIGHT][3];

  /* テクスチャの格納モード */
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  /* テクスチャ１の読み込み */
  if ((fp = fopen(texFile0, "rb")) == NULL) {
    perror(texFile0);
    exit(1);
  }
  fread(texImage, sizeof texImage, 1, fp);
  fclose(fp);

  glGenTextures(1, &texName0);
  glBindTexture(GL_TEXTURE_2D, texName0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXWIDTH, TEXHEIGHT, 0, GL_RGB,
    GL_UNSIGNED_BYTE, texImage);
  glBindTexture(GL_TEXTURE_2D, 0);

  /* テクスチャ２の読み込み */
  if ((fp = fopen(texFile1, "rb")) == NULL) {
    perror(texFile1);
    exit(1);
  }
  fread(texImage, sizeof texImage, 1, fp);
  fclose(fp);

  glGenTextures(1, &texName1);
  glBindTexture(GL_TEXTURE_2D, texName1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXWIDTH, TEXHEIGHT, 0, GL_RGB,
    GL_UNSIGNED_BYTE, texImage);
  glBindTexture(GL_TEXTURE_2D, 0);

  /* OpenGL の初期設定 */
  glClearColor(0.5, 0.5, 0.5, 0.0);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

#ifdef _WIN32
  /* GL_ARB_multitexture 用の関数ポインタの初期化 */
  if (!initMultiTexture()) {
    fprintf(stderr, "GL_ARB_multitexture が使用できません\n");
    exit(1);
  }
#endif
}

int main(int argc, char* argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH);
  glutCreateWindow(argv[0]);
  glutDisplayFunc(display);
  glutReshapeFunc(resize);
  glutKeyboardFunc(keyboard);
  init();
  glutMainLoop();

  return 0;
}
