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

#include <tspell/stringtrie.hh>
#include "testing.hh"

#define CHECK_FINDAPPROX(trie, sample, depth, expected) { \
		std::set<std::string> matches; \
		trie.FindApprox(sample, depth, matches); \
		if (matches.size() != 1) { \
			TEST_FAILED("FindApprox(\"" << sample << "\") returned " << matches.size() << " results (expected 1)" ); \
		} else if (*matches.begin() != expected) { \
			TEST_FAILED("FindApprox(\"" << sample << "\") returned \"" << *matches.begin() << "\" (expected \"" << expected << "\")" ); \
		} else { \
			TEST_PASSED("FindApprox(\"" << sample << "\") returned \"" << *matches.begin() << "\""); \
		} \
	}

#define CHECK_FINDAPPROX_NO(trie, sample, depth) { \
		std::set<std::string> matches; \
		trie.FindApprox(sample, depth, matches); \
		if (matches.empty()) { \
			TEST_PASSED("FindApprox(\"" << sample << "\") didn't return results" ); \
		} else { \
			TEST_FAILED("FindApprox(\"" << sample << "\") returned " << matches.size() << " results" ); \
		} \
	}

TEST_BEGIN()
	using TSpell::StringTrie;

	StringTrie trie;

	trie.Insert("ab");
	trie.Insert("testtesttest");

	/* exact */
	CHECK_FINDAPPROX(trie, "ab", 0, "ab");
	CHECK_FINDAPPROX(trie, "ab", 1, "ab");
	CHECK_FINDAPPROX(trie, "ab", 2, "ab");

	/* letter changed */
	CHECK_FINDAPPROX(trie, "xb", 1, "ab");
	CHECK_FINDAPPROX(trie, "ax", 1, "ab");
	CHECK_FINDAPPROX(trie, "xx", 2, "ab");

	CHECK_FINDAPPROX_NO(trie, "xx", 1);

	/* letter removed */
	CHECK_FINDAPPROX(trie, "a", 1, "ab");
	CHECK_FINDAPPROX(trie, "b", 1, "ab");

	CHECK_FINDAPPROX_NO(trie, "x", 1);

	/* letter added */
	CHECK_FINDAPPROX(trie, "cab", 1, "ab");
	CHECK_FINDAPPROX(trie, "acb", 1, "ab");
	CHECK_FINDAPPROX(trie, "abc", 1, "ab");

	CHECK_FINDAPPROX_NO(trie, "ccb", 1);
	CHECK_FINDAPPROX_NO(trie, "acc", 1);

TEST_END()
