// ######################################################################
// T. Nathan Mundhenk
// mundhenk@usc.edu
// C/C++ Macro HSV to RGB
#define PIX_HSV_TO_RGB_COMMON(H,S,V,R,G,B)                          \
if( V == 0 )                                                        \
{ R = 0; G = 0; B = 0; }                                            \
else if( S == 0 )                                                   \
{                                                                   \
  R = V;                                                            \
  G = V;                                                            \
  B = V;                                                            \
}                                                                   \
else                                                                \
{                                                                   \
  const double hf = H / 60.0;                                       \
  const int    i  = (int) floor( hf );                              \
  const double f  = hf - i;                                         \
  const double pv  = V * ( 1 - S );                                 \
  const double qv  = V * ( 1 - S * f );                             \
  const double tv  = V * ( 1 - S * ( 1 - f ) );                     \
  switch( i )                                                       \
    {                                                               \
    case 0:                                                         \
      R = V;                                                        \
      G = tv;                                                       \
      B = pv;                                                       \
      break;                                                        \
    case 1:                                                         \
      R = qv;                                                       \
      G = V;                                                        \
      B = pv;                                                       \
      break;                                                        \
    case 2:                                                         \
      R = pv;                                                       \
      G = V;                                                        \
      B = tv;                                                       \
      break;                                                        \
    case 3:                                                         \
      R = pv;                                                       \
      G = qv;                                                       \
      B = V;                                                        \
      break;                                                        \
    case 4:                                                         \
      R = tv;                                                       \
      G = pv;                                                       \
      B = V;                                                        \
      break;                                                        \
    case 5:                                                         \
      R = V;                                                        \
      G = pv;                                                       \
      B = qv;                                                       \
      break;                                                        \
    case 6:                                                         \
      R = V;                                                        \
      G = tv;                                                       \
      B = pv;                                                       \
      break;                                                        \
    case -1:                                                        \
      R = V;                                                        \
      G = pv;                                                       \
      B = qv;                                                       \
      break;                                                        \
    default:                                                        \
      abort();                                                      \
      break;                                                        \
    }                                                               \
}                                                                   \
R *= 255.0F;                                                        \
G *= 255.0F;                                                        \
B *= 255.0F;
