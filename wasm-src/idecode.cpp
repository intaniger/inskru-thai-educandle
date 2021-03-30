#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include "instruction.h"

struct HuffmanNode
{
  struct HuffmanNode *children[2];
  char leafVal;
};

class InstructionsDecoder
{
private:
  HuffmanNode *decoderRoot;
  void registerDefinition(char definition);

public:
  InstructionsDecoder(char definitions[], int length);
  std::vector<char> Decode(char *&bitstring);
  ~InstructionsDecoder();
};

void InstructionsDecoder::registerDefinition(char definition)
{
  char buffer = definition;
  char isActivated = 0x00;
  HuffmanNode *node = this->decoderRoot;
  for (size_t i = 0; i < 8; i++)
  {
    char bit = (buffer & 128) == 128; // if 128 then '1' else '0'
    buffer = (buffer << 1) & 255;
    if (bit)
      isActivated = 1;
    if (isActivated)
    {
      if (node->children[bit] == NULL)
        node->children[bit] = (HuffmanNode *)malloc(sizeof(HuffmanNode));
      node = node->children[bit];

      if (node->leafVal != NULL)
        throw "leafVal should not be assigned yet!";
    }
  }

  if (!isActivated)
  {
    node->children[0] = (HuffmanNode *)malloc(sizeof(HuffmanNode));
    node = node->children[0];
  }

  node->leafVal = definition;
}

InstructionsDecoder::InstructionsDecoder(char definitions[], int length)
{
  this->decoderRoot = (HuffmanNode *)malloc(sizeof(HuffmanNode));
  for (size_t i = 0; i < length; i++)
  {
    this->registerDefinition(definitions[i]);
  }
}

std::vector<char> InstructionsDecoder::Decode(char *&bitstring)
{
  char istn = NULL;
  HuffmanNode *node = this->decoderRoot;
  std::vector<char> instructions;

  while (istn != DRAW_CMD_EOF)
  {
    char buffer = *bitstring;
    for (size_t i = 0; i < 8; i++)
    {
      char bit = (buffer & 128) == 128;
      buffer = (buffer << 1) & 255;
      node = node->children[bit];

      if (node->children[0] == NULL && node->children[1] == NULL)
      {
        istn = node->leafVal;
        instructions.push_back(istn);
        node = this->decoderRoot;
        if (istn == DRAW_CMD_EOF)
          break;
      }
    }
    bitstring++;
  }
  return instructions;
}

InstructionsDecoder::~InstructionsDecoder()
{
}
