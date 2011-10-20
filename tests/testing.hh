/*
 * Copyright (C) 2011 Dmitry Marakasov
 *
 * This file is part of tspell.
 *
 * tspell is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * tspell is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with tspell.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <exception>

#define TEST_COLOR

#ifdef TEST_COLOR
#	define PASSED "\e[0;32mPASSED:\e[0m "
#	define FAILED "\e[1;31mFAILED:\e[0m "
#else
#	define PASSED "PASSED: "
#	define FAILED "FAILED: "
#endif

#define TEST_BEGIN() int main() { int ret = 0;
#define TEST_END() if (ret > 0) std::cerr << ret << " failures" << std::endl; return ret; }
#define TEST_FAILED(expr) do { std::cerr << FAILED << expr << std::endl; ++ret; } while(0)
#define TEST_PASSED(expr) do { std::cerr << PASSED << expr << std::endl; } while(0)
