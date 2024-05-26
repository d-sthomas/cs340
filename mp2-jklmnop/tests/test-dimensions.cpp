#include <cstring>

#include "lib/catch.hpp"
#include "../png-dimensions.h"

TEST_CASE("png_dimensions - 340.png", "[weight=1]") {
	uint32_t w, h;  
  system("cp tests/files/340.png TEST.png");
  int result = png_dimensions("TEST.png", &w, &h);
  
  CHECK( result == 0 );
  CHECK( w == 262 );
  CHECK( h == 146 );
  system("rm -f TEST.png");
}

TEST_CASE("png_dimensions - i.png", "[weight=1]") {
	uint32_t w, h;
  system("cp tests/files/i.png TEST.png");
  int result = png_dimensions("TEST.png", &w, &h);
  
  CHECK( result == 0 );
  CHECK( w == 686 );
  CHECK( h == 992 );
  system("rm -f TEST.png");
}

TEST_CASE("png_dimensions - 1989.png", "[weight=1]") {
	uint32_t w, h;
  system("cp tests/files/1989.png TEST.png");
  int result = png_dimensions("TEST.png", &w, &h);
  
  CHECK( result == 0 );
  CHECK( w == 926 );
  CHECK( h == 696 );
  system("rm -f TEST.png");
}
