#ifdef GLSLES
#ifdef FRAGMENT_SHADER
precision mediump float;
#endif
#else
#define highp
#define mediump
#define lowp
#endif