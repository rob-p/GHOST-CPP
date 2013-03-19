#include <cstdlib>
#include <memory>
#include <libgexf/libgexf.h>





int main( int argc, char** argv ) {

  std::unique_ptr<libgexf::FileReader> reader = std::unique_ptr<libgexf::FileReader>( new libgexf::FileReader() );
  reader->init( std::string(argv[1]) );
  reader->slurp();
  std::cout << "Hello world!\n";

  return 0;

}
