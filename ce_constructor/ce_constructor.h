#ifndef __CE_CONSTRUCTOR_H__
#define __CE_CONSTRUCTOR_H__

struct GraphChunk {
  int offset;
  int size;
  GraphChunk(int o=0, int s=0):offset(o), size(s){}
};

struct EdgeFlip {
  int count;
  int bestCount;
  int bestI;
  int bestJ;
};

#endif