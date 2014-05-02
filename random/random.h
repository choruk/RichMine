#ifndef __CE_CONSTRUCTOR_H__
#define __CE_CONSTRUCTOR_H__

struct GraphChunk {
  int offset;
  int size;
};

struct EdgeFlip {
  int count;
  int bestCount;
  int bestI;
  int bestJ;
};

#endif
