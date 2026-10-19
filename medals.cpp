// Program determines medal statistics based on data from standard input.

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <regex>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace {
using std::regex;
using std::vector;

using pattern_t = std::string;
using medal_t = uint8_t;
using year_t = uint16_t;
using competition_t = std::string;
using line_size_t = size_t;
using line_t = std::string;
using statistics_map_t = std::unordered_map<medal_t,
    std::unordered_map<year_t, std::unordered_map<competition_t, size_t>>>;

// Those are default values of medal, year and competition.
const medal_t EMPTY_MEDAL = 0;
const year_t EMPTY_YEAR = 0;
const competition_t EMPTY_COMPETITION = "";

// Medal is a number from 1 to 3.
const pattern_t MEDAL_PATTERN = R"(\b[123]\b)";

// Year is a number from 1800 to 59999.
const pattern_t YEAR_PATTERN = R"(\b(?:1[8-9]\d{2}|[2-9]\d{3}|[1-5]\d{4})\b)";

// Competition name is a sequence of words with english letters.
const pattern_t COMPETITION_PATTERN = R"(\b[a-zA-Z]+(?:[ \t]+[a-zA-Z]+)*\b)";

// Gap can consist of any positive number of spaces or tabulators.
const pattern_t GAP_PATTERN = R"([ \t]+)";

/* Query is a question mark or a question mark followed by gap and 
 * a combination of medal, year, competition that we ask about. */
const pattern_t QUERY_PATTERN = R"(\?|\?)" + GAP_PATTERN + R"((?:)";

// Function creates a string of all permutations of strings from a vector.
// Parameter patterns is a vector with patterns that build a valid input.
pattern_t permutePatterns(vector<pattern_t> patterns) {
    pattern_t permutedPatterns;

    /* next_permutation requires sorting as it transforms vectors
     * elements order into lexicographically next permutation, until it
     * reaches the lexicographically first permutation. */
    std::sort(patterns.begin(), patterns.end());

    // Add next permutation of patterns into possible input format.
    do {
        if (!permutedPatterns.empty())
            permutedPatterns += "|";

        for (size_t i = 0; i < patterns.size(); i++) {
            permutedPatterns += patterns[i];

            if (i < patterns.size() - 1)
                permutedPatterns += GAP_PATTERN;
        }
    } while (std::next_permutation(patterns.begin(), patterns.end()));

    return permutedPatterns;
}

// Functions creates a regex string consisting of all possible input formats.
pattern_t prepareInputValidator() {
    pattern_t patternWithoutQuestionMark =
        permutePatterns({MEDAL_PATTERN, YEAR_PATTERN, COMPETITION_PATTERN});

    vector<vector<pattern_t>> patternsCombinations = {
        {MEDAL_PATTERN},
        {YEAR_PATTERN},
        {COMPETITION_PATTERN},
        {MEDAL_PATTERN, YEAR_PATTERN},
        {MEDAL_PATTERN, COMPETITION_PATTERN},
        {YEAR_PATTERN, COMPETITION_PATTERN},
        {MEDAL_PATTERN, YEAR_PATTERN, COMPETITION_PATTERN}};

    // Regex has to have (?: to maintain ? at the beggining of patterns.
    pattern_t patternWithQuestionMark = QUERY_PATTERN;
    bool firstPermutation = true;

    for (const vector<pattern_t> &patternsCombination : patternsCombinations) {
        if (!firstPermutation)
            patternWithQuestionMark += "|";

        firstPermutation = false;

        patternWithQuestionMark += permutePatterns(patternsCombination);
    }

    // We need to close the opened (?:.
    patternWithQuestionMark += ")";

    /* Valid input consists of:
     * 1) some permutation of medal, year and competition name
     * 2) question mark and some permutation of not all from the above.
     * There can be any number of whitespaces between them. */
    return R"(^[ \t]*(?:)" + patternWithoutQuestionMark + "|" +
           patternWithQuestionMark + R"()[ \t]*$)";
}

// Function determines whether input line is valid by matching it with regex.
bool isLineValid(const line_t &line, const regex &inputValidator) {
    return std::regex_match(line, inputValidator);
}

// Function replaces multiple spaces and tabs with a single space.
// Parameter competition is a text that we want to normalize.
competition_t normalizeSpacesInCompetition(const competition_t &competition) {
    static const regex multipleSpaces(GAP_PATTERN);
    return std::regex_replace(competition, multipleSpaces, " ");
}

// Function extracts data from input line.
// Parameter line is a string from which we extract data.
// Returns a tuple with elements medal, year, competition.
std::tuple<medal_t, year_t, competition_t> extractData(const line_t &line) {
    // smatch is used for saving matched values from regex_search.
    std::smatch match;

    medal_t medal = EMPTY_MEDAL;
    year_t year = EMPTY_YEAR;
    competition_t competition = EMPTY_COMPETITION;

    // Modifier ensures that we compile this regex only once.
    // After successful search smatch has result at 0 index.
    static const regex regexMedalPattern(MEDAL_PATTERN);
    if (std::regex_search(line, match, regexMedalPattern))
        medal = std::stoi(match.str(0));

    static const regex regexYearPattern(YEAR_PATTERN);
    if (std::regex_search(line, match, regexYearPattern))
        year = std::stoi(match.str(0));

    static const regex regexCompetitionPattern(COMPETITION_PATTERN);
    if (std::regex_search(line, match, regexCompetitionPattern))
        competition = normalizeSpacesInCompetition(match.str(0));

    return std::make_tuple(medal, year, competition);
}

// Function updates medal statistics based on a valid input line.
// Parameter line is an input updating statistics.
// Parameter statistics is unordered_map that stores statistics.
void updateStats(const line_t &line, statistics_map_t &statistics) {
    auto [medal, year, competition] = extractData(line);

    statistics[medal][year][competition]++;

    /* To save statistics we update them also with default values so we
     * can answer queries with some of the medal, year, competition absent */
    statistics[medal][EMPTY_YEAR][EMPTY_COMPETITION]++;
    statistics[EMPTY_MEDAL][year][EMPTY_COMPETITION]++;
    statistics[EMPTY_MEDAL][EMPTY_YEAR][competition]++;

    statistics[medal][year][EMPTY_COMPETITION]++;
    statistics[medal][EMPTY_YEAR][competition]++;
    statistics[EMPTY_MEDAL][year][competition]++;

    statistics[EMPTY_MEDAL][EMPTY_YEAR][EMPTY_COMPETITION]++;
}

// Function answers query about statistics by printing them to standard output.
// Parameter line is a query.
// Parameter statistics is unordered_map that stores statistics
void answerQuery(const line_t &line, const statistics_map_t &statistics) {
    const auto [medal, year, competition] = extractData(line);

    auto iteratorMedal = statistics.find(medal);
    if (iteratorMedal == statistics.end()) {
        std::cout << 0 << '\n';
        return;
    }

    auto iteratorYear = iteratorMedal->second.find(year);
    if (iteratorYear == iteratorMedal->second.end()) {
        std::cout << 0 << '\n';
        return;
    }

    auto iteratorCompetition = iteratorYear->second.find(competition);
    if (iteratorCompetition == iteratorYear->second.end()) {
        std::cout << 0 << '\n';
        return;
    }

    std::cout << iteratorCompetition->second << '\n';
}
}

int main() {
    regex inputValidator(prepareInputValidator());

    statistics_map_t statistics;

    line_t line;
    line_size_t lineNumber = 1;

    while (std::getline(std::cin, line)) {
        if (isLineValid(line, inputValidator)) {
            if (line.find('?') != std::string::npos)
                answerQuery(line, statistics);
            else
                updateStats(line, statistics);
        }
        else {
            std::cerr << "ERROR " << lineNumber << '\n';
        }

        lineNumber++;
    }

    return 0;
}