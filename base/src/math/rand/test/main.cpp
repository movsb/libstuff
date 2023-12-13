#include <random.hpp>

#include <cstdio>

using namespace ebp::base::math::rand;

int main() {
	printf("int31: %d\n", int31_());
	printf("uint32: %d\n", uint32_());
	printf("int16: %d\n", int16_());
	printf("uint16: %d\n", uint16_());
}
