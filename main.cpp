#include <iostream>
#include <fstream>
#include <string>
#include <cctype>

// struct to hold sign, integer part, and fractional part separately
struct BigDouble {
    bool negative;             // true if sign is '-'
    std::string integerPart;   // digits before decimal point
    std::string fractionPart;  // digits after decimal point
};

// declarations
bool isValidDouble(const std::string &s);
BigDouble parseStringToBigDouble(const std::string &s);
BigDouble add(const BigDouble &a, const BigDouble &b);
BigDouble multiply(const BigDouble &a, const BigDouble &b);
std::string bigDoubleToString(const BigDouble &bd);

// removes leading zeros in integer part and trailing zeros in fraction part
void normalize(BigDouble &bd);

int main() {
    // asks user for an input file name
    std::string filename;
    std::cout << "Enter filename: ";
    std::cin >> filename;

    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        std::cerr << "Could not open file.\n";
        return 1;
    }

    // prepares the constants being added/multiplied by (-123.456) and (123.456)
    BigDouble addConst  = parseStringToBigDouble("-123.456");
    BigDouble multConst = parseStringToBigDouble("123.456");

    std::string line;
    while (std::getline(inFile, line)) {
        // checks if the line is a valid double string
        if (!isValidDouble(line)) {
            std::cout << "Invalid number: " << line << std::endl;
            continue;
        }

        // parses the line into a BigDouble
        BigDouble number = parseStringToBigDouble(line);

        // adds
        BigDouble sumResult = add(number, addConst);

        // multiplies
        BigDouble multResult = multiply(number, multConst);

        // prints results
        std::cout << line << " + (-123.456) = "
                  << bigDoubleToString(sumResult) << std::endl;
        std::cout << line << " * 123.456   = "
                  << bigDoubleToString(multResult) << std::endl;
    }

    return 0;
}

// checks if 's' is a valid decimal string with optional sign and optional decimal point
bool isValidDouble(const std::string &s) {
    if (s.empty()) return false;

    int i = 0;
    int n = s.size();

    // sign
    if (s[i] == '+' || s[i] == '-') {
        i++;
    }

    bool hasDigits = false;
    // digits before decimal point
    while (i < n && std::isdigit(s[i])) {
        hasDigits = true;
        i++;
    }

    // optional decimal point (if present, must have digits after)
    if (i < n && s[i] == '.') {
        i++;
        bool hasFractionDigits = false;
        while (i < n && std::isdigit(s[i])) {
            hasFractionDigits = true;
            i++;
        }
        // if decimal point is present, ensure at least one digit followed
        if (!hasFractionDigits) {
            return false;
        }
        hasDigits = true;  // do have some digits overall
    }

    // should be at the end of the string if it's valid and ensure we had at least one digit
    return (hasDigits && i == n);
}

// parses a validated decimal string into a BigDouble struct (sign, integerPart, fractionPart). 
BigDouble parseStringToBigDouble(const std::string &s) {
    BigDouble result;
    result.negative     = false;
    result.integerPart  = "0";
    result.fractionPart = "";

    int i = 0;
    int n = s.size();

    // optional sign
    if (s[i] == '+' || s[i] == '-') {
        if (s[i] == '-') {
            result.negative = true;
        }
        i++;
    }

    // reads integer part (digits until decimal point or end)
    while (i < n && std::isdigit(s[i])) {
        result.integerPart.push_back(s[i]);
        i++;
    }

    // if there's a decimal point, read the fraction part
    if (i < n && s[i] == '.') {
        i++;  // skip '.'
        while (i < n && std::isdigit(s[i])) {
            result.fractionPart.push_back(s[i]);
            i++;
        }
    }

    // normalizes to remove extra zeros
    normalize(result);

    return result;
}

// converts a BigDouble struct back to a string (with sign, decimal point if needed).
std::string bigDoubleToString(const BigDouble &bd) {
    // handles special case: if integerPart is "0" and fractionPart is "", it's zero
    if (bd.integerPart == "0" && bd.fractionPart.empty()) {
        return "0";
    }

    std::string out;
    if (bd.negative && !(bd.integerPart == "0" && bd.fractionPart.empty())) {
        out.push_back('-');
    }
    out += bd.integerPart;

    if (!bd.fractionPart.empty()) {
        out.push_back('.');
        out += bd.fractionPart;
    }
    return out;
}

// normalizes the BigDouble by removing unnecessary leading zeros in integerPart and trailing zeros in fractionPart. Also handle the "all zeros" case.
void normalize(BigDouble &bd) {
    // removes leading zeros in integer part but leave at least one digit if all are zeros
    int start = 0;
    while (start + 1 < (int)bd.integerPart.size() &&
           bd.integerPart[start] == '0') {
        start++;
    }
    bd.integerPart.erase(0, start);

    if (bd.integerPart.empty()) {
        // if everything is cleared, integerPart should be "0"
        bd.integerPart = "0";
    }

    // removes trailing zeros in fractional part
    while (!bd.fractionPart.empty() && bd.fractionPart.back() == '0') {
        bd.fractionPart.pop_back();
    }

    // if integerPart == "0" and fractionPart is empty, it's effectively 0
    if (bd.integerPart == "0" && bd.fractionPart.empty()) {
        bd.negative = false;  // -0 becomes +0
    }
}

// string-based addition of two BigDouble values.
BigDouble add(const BigDouble &a, const BigDouble &b) {

    // helper lambdas
    auto absGreater = [&](const BigDouble &x, const BigDouble &y) {
        // compare length of integerPart
        if (x.integerPart.size() != y.integerPart.size()) {
            return (x.integerPart.size() > y.integerPart.size());
        }
        // compares integerPart lexicographically
        if (x.integerPart != y.integerPart) {
            return (x.integerPart > y.integerPart);
        }
        // compares fractionPart length or lex
        if (x.fractionPart.size() != y.fractionPart.size()) {
            // the one with longer fraction might be bigger if the shorter fraction is a prefix. But we must compare lexicographically up to min length
            size_t minLen = std::min(x.fractionPart.size(), y.fractionPart.size());
            std::string xPart = x.fractionPart.substr(0, minLen);
            std::string yPart = y.fractionPart.substr(0, minLen);
            if (xPart != yPart) {
                return (xPart > yPart);
            }
            // if they are same up to minLen, then the longer one with a non-zero next digit is bigger
            return (x.fractionPart.size() > y.fractionPart.size());
        }
        // compares fractionPart lexicographically
        return (x.fractionPart > y.fractionPart);
    };

    // helper for absolute addition (both positive, ignoring sign)
    auto addAbs = [&](const BigDouble &x, const BigDouble &y) {
        // aligns fraction parts (pad the shorter fraction with trailing zeros)
        BigDouble res;
        res.negative = false;

        // determines max fraction length
        size_t maxFrac = std::max(x.fractionPart.size(), y.fractionPart.size());
        std::string fracX = x.fractionPart;
        std::string fracY = y.fractionPart;
        fracX.append(maxFrac - fracX.size(), '0');
        fracY.append(maxFrac - fracY.size(), '0');

        // adds the fraction part from right to left
        int carry = 0;
        std::string fracSum;
        for (int i = (int)maxFrac - 1; i >= 0; i--) {
            int digitX = fracX[i] - '0';
            int digitY = fracY[i] - '0';
            int sum    = digitX + digitY + carry;
            carry      = sum / 10;
            sum       %= 10;
            fracSum.push_back((char)('0' + sum));
        }
        // Reverse fracSum
        std::reverse(fracSum.begin(), fracSum.end());
        res.fractionPart = fracSum;

        // add the integer parts
        std::string intX = x.integerPart;
        std::string intY = y.integerPart;
        // pad to the same length
        if (intX.size() < intY.size()) {
            intX.insert(intX.begin(), intY.size() - intX.size(), '0');
        } else if (intY.size() < intX.size()) {
            intY.insert(intY.begin(), intX.size() - intY.size(), '0');
        }

        std::string intSum;
        for (int i = (int)intX.size() - 1; i >= 0; i--) {
            int digitX = intX[i] - '0';
            int digitY = intY[i] - '0';
            int sum    = digitX + digitY + carry;
            carry      = sum / 10;
            sum       %= 10;
            intSum.push_back((char)('0' + sum));
        }
        if (carry) {
            intSum.push_back((char)('0' + carry));
        }
        // reverses intSum
        std::reverse(intSum.begin(), intSum.end());
        res.integerPart = intSum;

        // normalizes
        normalize(res);
        return res;
    };

    // helper for absolute subtraction: x - y, assuming x >= y in absolute value
    auto subAbs = [&](const BigDouble &x, const BigDouble &y) {
        // we'll do fraction alignment as well, then subtract
        BigDouble res;
        res.negative = false;

        // aligns fraction length
        size_t maxFrac = std::max(x.fractionPart.size(), y.fractionPart.size());
        std::string fracX = x.fractionPart;
        std::string fracY = y.fractionPart;
        fracX.append(maxFrac - fracX.size(), '0');
        fracY.append(maxFrac - fracY.size(), '0');

        // subtracts fraction part
        int borrow = 0;
        std::string fracDiff;
        for (int i = (int)maxFrac - 1; i >= 0; i--) {
            int digitX = fracX[i] - '0';
            int digitY = fracY[i] - '0';
            digitX -= borrow;
            borrow = 0;
            if (digitX < digitY) {
                digitX += 10;
                borrow = 1;
            }
            int diff = digitX - digitY;
            fracDiff.push_back((char)('0' + diff));
        }
        std::reverse(fracDiff.begin(), fracDiff.end());
        res.fractionPart = fracDiff;

        // subtracts integer part
        std::string intX = x.integerPart;
        std::string intY = y.integerPart;
        if (intX.size() < intY.size()) {
            intX.insert(intX.begin(), intY.size() - intX.size(), '0');
        } else if (intY.size() < intX.size()) {
            intY.insert(intY.begin(), intX.size() - intY.size(), '0');
        }

        std::string intDiff;
        for (int i = (int)intX.size() - 1; i >= 0; i--) {
            int digitX = intX[i] - '0';
            int digitY = intY[i] - '0';
            digitX -= borrow;
            borrow = 0;
            if (digitX < digitY) {
                digitX += 10;
                borrow = 1;
            }
            int diff = digitX - digitY;
            intDiff.push_back((char)('0' + diff));
        }
        // removes leading zeros from intDiff after we reverse it
        std::reverse(intDiff.begin(), intDiff.end());
        while (intDiff.size() > 1 && intDiff[0] == '0') {
            intDiff.erase(intDiff.begin());
        }

        res.integerPart = intDiff;
        // now normalize to remove trailing zeros in fraction part etc.
        normalize(res);
        return res;
    };

    // we now handle the sign logic:
    BigDouble result;
    if (a.negative == b.negative) {
        // same sign => absolute addition
        result = addAbs(a, b);
        // if both are negative, result is negative
        result.negative = a.negative; // (which is the same as b.negative)
    } else {
        // opposite signs => subtract, and figures out which is bigger in absolute value
        if (absGreater(a, b)) {
            // a has bigger absolute value => result sign = a's sign
            result = subAbs(a, b);
            result.negative = a.negative;
        } else {
            // b has bigger absolute value => result sign = b's sign
            result = subAbs(b, a);
            result.negative = b.negative;
        }
    }

    // final normalize
    normalize(result);
    return result;
}

// string-based multiplication of two BigDouble values.
BigDouble multiply(const BigDouble &a, const BigDouble &b) {

    // counts fraction digits
    int totalFracDigits = (int)(a.fractionPart.size() + b.fractionPart.size());

    // creates big integer strings ignoring sign
    std::string bigA = a.integerPart + a.fractionPart;
    std::string bigB = b.integerPart + b.fractionPart;

    // removes leading zeros from those combined strings if any
    while (bigA.size() > 1 && bigA[0] == '0') {
        bigA.erase(bigA.begin());
    }
    while (bigB.size() > 1 && bigB[0] == '0') {
        bigB.erase(bigB.begin());
    }

    // if either is "0", the result is 0
    if (bigA == "0" || bigB == "0" || bigA.empty() || bigB.empty()) {
        // returns 0
        BigDouble zero;
        zero.negative = false;
        zero.integerPart = "0";
        zero.fractionPart = "";
        return zero;
    }

    // multiplies bigA and bigB as large integers and store the result in a vector of digits
    std::string res(bigA.size() + bigB.size(), '0');

    for (int i = (int)bigA.size() - 1; i >= 0; i--) {
        int carry = 0;
        int n1 = bigA[i] - '0';
        for (int j = (int)bigB.size() - 1; j >= 0; j--) {
            int n2 = bigB[j] - '0';
            int sum = (res[i + j + 1] - '0') + n1 * n2 + carry;
            carry  = sum / 10;
            res[i + j + 1] = (char)('0' + (sum % 10));
        }
        res[i] = (char)('0' + ((res[i] - '0') + carry));
    }

    // removes leading '0's
    while (res.size() > 1 && res[0] == '0') {
        res.erase(res.begin());
    }

    // inserts decimal point in the correct position
    // We have 'res' as an integer. If totalFracDigits > 0, we need to split it.
    BigDouble result;
    result.negative = (a.negative != b.negative); // sign is negative if exactly one is negative

    // if totalFracDigits >= res.size(), it means the decimal point goes to the left of all digits
    if (totalFracDigits >= (int)res.size()) {
        // We need to pad with leading zeros
        int padCount = totalFracDigits - (int)res.size();
        std::string fractionPart = std::string(padCount, '0') + res; // e.g. "0012"

        // integer part is "0"
        result.integerPart = "0";
        result.fractionPart = fractionPart;
    } else {
        // normal case: place the decimal point
        int pointPos = (int)res.size() - totalFracDigits;
        result.integerPart  = res.substr(0, pointPos);
        result.fractionPart = res.substr(pointPos);
    }

    // normalizes (remove trailing zeros in fraction and leading zeros in integer)
    normalize(result);
    return result;
}
