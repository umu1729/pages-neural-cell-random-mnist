#include <emscripten/emscripten.h>
#include <tgmath.h>

#include "rev2.param.h"
#include <stdlib.h>
const int HALF_MAX = RAND_MAX/2;

extern "C" float EMSCRIPTEN_KEEPALIVE add314(float a,float b){
  return a+b;
}

extern "C" float EMSCRIPTEN_KEEPALIVE arr_add1(float* a, int length){
  for (int i=0;i<length;i++){
    a[i]+=1.;
  }
  return 0;
}

extern "C" float EMSCRIPTEN_KEEPALIVE arr_addui( unsigned char* a, int value ,int length){
  for (int i=0;i<length;i++){
    a[i]+=value;
  }
  return 0;
}

int wrap(int v,int m){
  if (v<0) v += m;
  if (v>=m) v-=m;
  return v;
}

extern "C" float EMSCRIPTEN_KEEPALIVE update_state( float* arr, float* hid, int Sz, int Ch){
  for (int y=0;y<Sz;y++){
    for (int x=0;x<Sz;x++){
      for (int co=0;co<64;co++){
        float v = cv1b[co];
        for (int ci=0;ci<16;ci++){
          for (int h=0;h<3;h++){
            for(int w=0;w<3;w++){
              //buf += cv1w[co,ci,h,w] * st[ci,(x+h-1)%Sz,(y+w-1)%Sz]
              // cv1w=(64,16,3,3) st=(Sz,Sz,16)
              v += cv1w[co*16*3*3+ci*3*3+h*3+w] * arr[wrap(x+h-1,Sz)*Sz*16+wrap(y+w-1,Sz)*16+ci];
            }
          }
        }
        // hid = (Sz,Sz,64)
        hid[x*Sz*64+y*64+co] =  (v>0.)?v:0.;
        //int i = y*Sz*Ch + x*Ch + c;
      }
    }
  }

  for (int y=0;y<Sz;y++){
    for (int x=0;x<Sz;x++){
      if ( rand()>HALF_MAX ){
        continue;
      }
      for (int co=0;co<16;co++){
        if (co==10 || co==11){
          continue;
        }
        float v = arr[x*Sz*16+y*16+co]+cv2b[co];
        for (int ci=0;ci<64;ci++){
          for (int h=0;h<3;h++){
            for(int w=0;w<3;w++){
              //buf += cv1w[co,ci,h,w] * st[ci,(x+h-1)%Sz,(y+w-1)%Sz]
              // cv1w=(16,64,3,3) st=(Sz,Sz,16)
              v += cv2w[co*64*3*3+ci*3*3+h*3+w] * hid[wrap(x+h-1,Sz)*Sz*64+wrap(y+w-1,Sz)*64+ci];
            }
          }
        }
        // hid = (Sz,Sz,64)
        arr[x*Sz*16+y*16+co] =  v;
        //int i = y*Sz*Ch + x*Ch + c;
      }
    }
  }

  return 0;
}

extern "C" float EMSCRIPTEN_KEEPALIVE init_state( float* arr, int Sz, int Ch){
  for (int y=0;y<Sz;y++){
    for (int x=0;x<Sz;x++){
      for (int c=0;c<Ch;c++){
        int i = y*Sz*Ch + x*Ch + c;
        arr[i] = -1;
      }
    }
  }
  return 0;
}

extern "C" float EMSCRIPTEN_KEEPALIVE set_float( float* arr, int Sz, int Ch, int x, int y, int c,int v){
  int i = y*Sz*Ch + x*Ch + c;
  arr[i] = v;
  return 0;
}
extern "C" float EMSCRIPTEN_KEEPALIVE get_float( float* arr, int Sz, int Ch, int x, int y, int c){
  int i = y*Sz*Ch + x*Ch + c;
  return arr[i];
}

extern "C" float EMSCRIPTEN_KEEPALIVE state2img( float* arr, int Sz, int Ch, unsigned char* img){
  for (int y=0;y<Sz;y++){
    for (int x=0;x<Sz;x++){
      for (int c=0;c<4;c++){
        int i = y*Sz*4 + x*4 + c;
        if (c==3) {
          img[i] = 255;
        }else{
          float v = arr[y*Sz*Ch+x*Ch+0]*.5+.5;
          v = v<0.?0.:v;
          v = v>1.?1.:v;
          img[i] = (int)(v*255.);
        }
      }
    }
  }
  return 0;
}
