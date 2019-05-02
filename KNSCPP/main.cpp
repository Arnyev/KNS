#include <iostream>
#include <array>
#include <unordered_map>

typedef uint8_t u8;
typedef uint64_t u64;

constexpr int player_a_won = 1;
constexpr int player_b_won = 0;
constexpr u8 alphabet_size = 5;
constexpr size_t max_word_length = 16;
constexpr int char_bits = 3;
constexpr u64 bit_mask = 7;

std::array<int, max_word_length + 1> counts;
std::unordered_map<u64, int> already_analysed;
u64 min_value_calls = 0;

int max_value(u64 word, int len, int move_index);

constexpr u64 invert(const u64 value, const int len)
{
    u64 new_val = 0;
    for (int i = 0; i < len; i++)
        new_val |= (value >> (len - i - 1)*char_bits & bit_mask) << i * char_bits;

    return new_val;
}

constexpr u64 get_base_permutation(const u64 value, const int len)
{
    u8 new_chars[alphabet_size + 1]{};

    int char_index = 1;
    for (int i = 0; i < len; i++)
    {
        const auto c = value >> i * char_bits & bit_mask;
        if (new_chars[c] == 0)
            new_chars[c] = char_index++;
    }

    u64 new_value = 0;
    for (int i = len - 1; i >= 0; i--)
        new_value = (new_value << char_bits) | new_chars[(value >> (i * char_bits)) & bit_mask];

    return new_value;
}

std::string to_string(u64 word, int len)
{
    std::string str;
    for (int i = 0; i < len; i++)
        str.insert(str.begin(), (word >> char_bits * i &bit_mask) + 'a' - 1);

    return str;
}

void from_string(const std::string& str, u64& value, int& len)
{
    value = 0;
    len = static_cast<int>(str.size());
    for (auto i : str)
        value = value << char_bits | i + 1 - 'a';
}

int main()
{
    for (auto& i : counts)
        i = 0;

    const int utility = max_value(0, 0, 0);

    std::cout << (utility == player_a_won ? "player A won" : "player B won") << '\n';
    std::cout << "analysed strings count " << already_analysed.size() << '\n';
    auto index = 0;
    for (auto i : counts)
        std::cout << "String Count size " << index++ << " : " << i << '\n';

    std::cout << "Min value calls : " << min_value_calls << '\n';
    int sq;
    std::cin >> sq;
}


constexpr bool is_square_free(u64 value, const int len)
{
    std::array<u8, max_word_length> s{};
    for (int i = 0; i < len; i++)
    {
        s[i] = value & bit_mask;
        value >>= char_bits;
    }

    for (int l = 1; 2 * l - 1 <= len / 2; l *= 2)
    {
        int x[2] = { -1, -1 };
        const auto blockEnd = len - 3 * l + 2;

        for (int i = 0; i < blockEnd; i += l)
        {
            int xIndex = 0;

            int pos = i + 2 * l - 1;
            auto lastPos = i + 4 * l - 2;

            if (lastPos > len - l)
                lastPos = len - l;

            while (pos <= lastPos)
            {
                int index = 0;
                bool broken = false;
                for (; index < l; index++)
                    if (s[i + index] != s[pos + index])
                    {
                        broken = true;
                        break;
                    }
                if (!broken)
                {
                    x[xIndex++] = pos;
                    if (xIndex == 2)
                        break;
                }
                pos += index + 1;
            }

            for (int j = 0; j < xIndex; j++)
            {
                const auto repeatBlockStart = x[j];

                int left = 0;
                while (i - 1 - left >= 0 && s[i - 1 - left] == s[repeatBlockStart - 1 - left])
                    left++;

                int right = l;
                while (repeatBlockStart + right < len && s[i + right] == s[repeatBlockStart + right])
                    right++;

                if (left + right >= repeatBlockStart - i)
                    return false;
            }
        }
    }

    return true;
}

constexpr bool is_finished(const u64 word, const int len, int& utility)
{
    const auto sqfree = is_square_free(word, len);
    if (sqfree)
    {
        if (len == max_word_length)
        {
            utility = player_a_won;
            return true;
        }
        return false;
    }

    utility = player_b_won;
    return true;
}

int min_value(const u64 word, const int len)
{
    min_value_calls++;

    const auto key = get_base_permutation(word, len);
    const auto it = already_analysed.find(key);
    if (it != already_analysed.end())
        return it->second;

    const auto inverted = invert(word, len);
    const auto inverted_key = get_base_permutation(inverted, len);

    const auto it2 = already_analysed.find(inverted_key);
    if (it2 != already_analysed.end())
        return it2->second;

    int utility;
    if (is_finished(word, len, utility))
        return utility;

    utility = player_a_won;
    for (int i = 0; i <= len; i++)
    {
        if (max_value(word, len, i) == player_b_won)
        {
            utility = player_b_won;
            break;
        }
    }

    counts[len]++;
    already_analysed.insert({ key, utility });

    return utility;
}

int max_value(const u64 word, const int len, const int move_index)
{
    constexpr u64 base_mask = ~0ull;
    const u64 erase_right_mask = base_mask << (char_bits*move_index + char_bits);
    const u64 right_bits_mask = ~(base_mask << (char_bits*move_index));

    const u64 left_bits = (word << char_bits) & erase_right_mask;
    const u64 right_bits = word & right_bits_mask;
    u64 new_word = left_bits | right_bits;

    const u64 erase_mask = ~(bit_mask << (char_bits*move_index));

    for (u64 i = 1; i <= alphabet_size; i++)
    {
        new_word &= erase_mask;
        new_word |= i << (move_index*char_bits);

        if (min_value(new_word, len + 1) == player_a_won)
            return player_a_won;
    }

    return player_b_won;
}
