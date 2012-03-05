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

#include <fcntl.h>
#include <errno.h>
#include <locale.h>
#include <string.h>
#include <stdio.h>

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <list>
#include <stdexcept>
#include <cassert>

#include <tspell/stringtrie.hh>

static std::wstring char2wstring(const char* str) {
	size_t buflen = strlen(str) + 1;
	wchar_t* buffer = new wchar_t[buflen];
	if (mbstowcs(buffer, str, buflen) == (size_t)-1) {
		delete[] buffer;
		throw std::runtime_error("incorrect myltibyte sequence");
	}
	buffer[buflen - 1] = L'\0';
	return std::wstring(buffer);
}

class FileProcessor {
private:
	struct WordLocation {
		int file;
		int line;
		int bytepos;
		int charpos;

		WordLocation(int f, int l, int bp, int cp) : file(f), line(l), bytepos(bp), charpos(cp) {
		}
	};

	struct WordInfo {
		typedef std::vector<WordLocation> LocationsVector;

		int count;
		LocationsVector locations;

		WordInfo() : count(0) {
		}
	};

private:
	typedef std::vector<std::wstring> FilenameVector;
	typedef std::map<std::wstring, WordInfo> WordMap;

private:
	FilenameVector files_;
	WordMap words_;

	int typo_threshold_;
	int correct_threshold_;
	int min_word_length_;
	int distance_;
	std::wstring wordchars_;

private:
	void AddWord(const std::wstring& word, const WordLocation& location) {
		if (word.length() < (size_t)min_word_length_)
			return;

		WordInfo& winfo = words_[word];
		++winfo.count;

		/* if a word is common enough, forget location info for it */
		if (winfo.count == correct_threshold_) {
			std::vector<WordLocation> dummy;
			winfo.locations.swap(dummy);
		} else if (winfo.count < correct_threshold_) {
			winfo.locations.push_back(location);
		}
	}

public:
	FileProcessor(int typo_threshold, int correct_threshold, int min_word_length, int distance, const std::wstring& wordchars)
		: typo_threshold_(typo_threshold),
		  correct_threshold_(correct_threshold),
		  min_word_length_(min_word_length),
		  distance_(distance),
		  wordchars_(wordchars) {
	}

	void ProcessFile(const char* path) {
		files_.push_back(char2wstring(path));

		int f;
		if ((f = open(path, O_RDONLY)) == -1)
			throw std::runtime_error(std::string("Cannot open file: ") + strerror(errno));

		char buffer[1024];
		ssize_t nread;

		std::wstring word;
		WordLocation location(files_.size() - 1, 1, 1, 1);
		WordLocation startword = location;

		mbstate_t ps;
		memset(&ps, 0, sizeof(ps));

		wchar_t currchar;
		size_t rval;

		while ((nread = read(f, buffer, sizeof(buffer))) > 0) {
			char *cur = buffer;
			size_t len = nread;

			while (len > 0) {
				rval = mbrtowc(&currchar, cur, len, &ps);
				if (rval == (size_t)-1) {
					std::wcerr << files_.back() << L":" << location.line << L":" << location.bytepos << ": incorrect multibyte sequence" << std::endl;
					cur++;
					len--;
					continue;
				} else if (rval == (size_t)-2) {
					location.bytepos += len;
					break; /* refill buffer and continue processing */
				} else if (rval == 0) {
					rval = 1;
				}

				cur += rval;
				len -= rval;

				if (iswalpha(currchar) || wordchars_.find(currchar) != std::wstring::npos) {
					/* word character */
					if (word.empty())
						startword = location;
					word += currchar;
				} else {
					/* space character */
					if (!word.empty()) {
						/* if there was a word before it, process it */
						AddWord(word, startword);
						word.clear();
					}
				}

				if (currchar == L'\n') {
					/* if there was a newline, update location */
					location.line++;
					location.bytepos = 1;
					location.charpos = 1;
				} else {
					location.charpos++;
					location.bytepos += rval;
				}
			}
		}

		if (!word.empty())
			AddWord(word, location);

		if (nread == -1)
			throw std::runtime_error(std::string("Read error: ") + strerror(errno));

		close(f);
	}

	void DumpResults() {
		TSpell::WStringTrie spelltree;

		/* feed the tree */
		for (WordMap::const_iterator word = words_.begin(); word != words_.end(); ++word)
			if (word->second.count >= correct_threshold_)
				spelltree.Insert(word->first);

		/* process all words that are rare enough to be spelling errors */
		for (WordMap::const_iterator word = words_.begin(); word != words_.end(); ++word) {
			if (word->second.count <= typo_threshold_) {
				/* get all possible corrections from trie */
				std::set<std::wstring> corrections;
				spelltree.FindApprox(word->first, distance_, corrections);

				/* postprocess corrections */
				for (std::set<std::wstring>::iterator c = corrections.begin(); c != corrections.end();) {
					bool keep = true;
					if (wcscasecmp(c->c_str(), word->first.c_str()) == 0)
						keep = false;

					if (keep) {
						++c;
					} else {
						std::set<std::wstring>::iterator tmp = c++;
						corrections.erase(tmp);
					}
				}

				if (corrections.empty())
					continue;

				/* display */
				std::wcout << "\"" << word->first << "\"" << std::endl;
				std::wcout << "\t" << word->second.count << " occurrences:" << std::endl;
				for (WordInfo::LocationsVector::const_iterator loc = word->second.locations.begin();
						loc != word->second.locations.end(); ++loc)
					std::wcout << "\t\t" << files_[loc->file] << ":" << loc->line << " byte " << loc->bytepos  << " char " << loc->charpos << std::endl;
				std::cout << "\t" << corrections.size() << " possible corrections:" << std::endl;
				for (std::set<std::wstring>::const_iterator corr = corrections.begin(); corr != corrections.end(); ++corr) {
					WordMap::const_iterator corrword = words_.find(*corr);
					assert(corrword != words_.end());

					std::wcout << "\t\t" << *corr << " (" << corrword->second.count << ")" << std::endl;
				}
			}
		}
	}
};

int usage(const char* progname, int code) {
    fprintf(stderr, "Usage: %s [options] file ...\n", progname);
    fprintf(stderr, "  -t  set max number of occurrences for misspelled words (default 1)\n");
    fprintf(stderr, "  -c  set min number of occurrences for correct words (default 10)\n");
    fprintf(stderr, "  -l  set minimum length of a word (default 3)\n");
    fprintf(stderr, "  -w  specify an additional set of characters to be considered\n");
    fprintf(stderr, "      as word letters (which are only alphabetic by default)\n");
    fprintf(stderr, "  -d  specify maximum number of changes in a word (default 1)\n");

    fprintf(stderr, "  -h  display this help\n");
    return code;
}

int main(int argc, char** argv) {
	setlocale(LC_ALL, "");

    const char* progname = argv[0];

	int distance = 1;
	int typo_threshold = 1;
	int correct_threshold = 10;
	int min_word_length = 3;

	std::wstring wordchars;

    int c;
    while ((c = getopt(argc, argv, "t:c:d:l:w:h")) != -1) {
        switch (c) {
            case 't': typo_threshold = (int)strtol(optarg, 0, 10); break;
            case 'c': correct_threshold = (int)strtol(optarg, 0, 10); break;
            case 'l': min_word_length = (int)strtol(optarg, 0, 10); break;
            case 'w': wordchars = char2wstring(optarg); break;
			case 'd': distance = (int)strtol(optarg, 0, 10); break;
            case 'h': return usage(progname, 0);
            default:
                return usage(progname, 1);
        }
    }

    argc -= optind;
    argv += optind;

	FileProcessor processor(typo_threshold, correct_threshold, min_word_length, distance, wordchars);

	for (; argc > 0; --argc, ++argv)
		processor.ProcessFile(*argv);

	processor.DumpResults();

	return 0;
}
