#include "tecplotread.hpp"

// this is a really simple main file
int main(int argc, char* arcg[])
{
  assert(argc == 2);

  std::string filetp(arcg[1]);

  shared_ptr<tecplotread> tpobj = make_shared<tecplotread>(filetp);

  tpobj->complete_information();

  return 0;
}
