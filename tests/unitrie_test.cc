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

#include <tspell/unitrie.hh>
#include "testing.hh"

#define CHECK_FINDAPPROX(trie, sample, depth, expected) { \
		std::set<UnicodeString> matches; \
		trie.FindApprox(UnicodeString::fromUTF8(sample), depth, matches); \
		if (matches.size() != 1) { \
			TEST_FAILED("FindApprox(" << sample << ") returned " << matches.size() << " results (expected 1)" ); \
		} else if (*matches.begin() != UnicodeString::fromUTF8(expected)) { \
			std::string temp; \
			matches.begin()->toUTF8String(temp); \
			TEST_FAILED("FindApprox(" << sample << ") returned " << temp << " (expected " << expected << ")" ); \
		} else { \
			std::string temp; \
			matches.begin()->toUTF8String(temp); \
			TEST_PASSED("FindApprox(" << sample << ") returned " << temp); \
		} \
	}

#define CHECK_FINDAPPROX_NO(trie, sample, depth) { \
		std::set<UnicodeString> matches; \
		trie.FindApprox(UnicodeString::fromUTF8(sample), depth, matches); \
		if (matches.empty()) { \
			TEST_PASSED("FindApprox(" << sample << ") didn't return results" ); \
		} else { \
			TEST_FAILED("FindApprox(" << sample << ") returned " << matches.size() << " results" ); \
		} \
	}

TEST_BEGIN()
	using TSpell::UnicodeTrie;

	UnicodeTrie trie;

	trie.Insert(UnicodeString::fromUTF8("мама мыла раму"));

	/* exact */
	CHECK_FINDAPPROX(trie, "мама мыла раму", 0, "мама мыла раму");
	CHECK_FINDAPPROX(trie, "мама мыла раму", 1, "мама мыла раму");
	CHECK_FINDAPPROX(trie, "мама мыла раму", 2, "мама мыла раму");

	/* letter changed */
	CHECK_FINDAPPROX(trie, "мама мыло раму", 1, "мама мыла раму");
	CHECK_FINDAPPROX(trie, "мама мыла рамы", 1, "мама мыла раму");
	CHECK_FINDAPPROX(trie, "лама мыла раму", 1, "мама мыла раму");
	CHECK_FINDAPPROX(trie, "лама мыла рамы", 2, "мама мыла раму");

	CHECK_FINDAPPROX_NO(trie, "лама мыла рамы", 1);

	/* letter added */
	CHECK_FINDAPPROX(trie, "амама мыла раму", 1, "мама мыла раму");
	CHECK_FINDAPPROX(trie, "мама мылла раму", 1, "мама мыла раму");
	CHECK_FINDAPPROX(trie, "мама мыла рамуп", 1, "мама мыла раму");

	/* letter removed */
	CHECK_FINDAPPROX(trie, "ама мыла раму", 1, "мама мыла раму");
	CHECK_FINDAPPROX(trie, "мама мыла рам", 1, "мама мыла раму");
	CHECK_FINDAPPROX(trie, "мама мла раму", 1, "мама мыла раму");

	/* many */
	CHECK_FINDAPPROX(trie, "папа жгла газу", 6, "мама мыла раму");
	CHECK_FINDAPPROX_NO(trie, "папа жгла газу", 5);

TEST_END()
