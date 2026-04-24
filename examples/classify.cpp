// Read one address per line from stdin and print the RFC range
// classification for each.
//
//   $ printf '127.0.0.1\n10.0.0.1\n2001:db8::1\n' | ./classify
//   127.0.0.1       loopback
//   10.0.0.1        private
//   2001:db8::1     reserved

#include <iostream>
#include <string>
#include <variant>

#include <polycpp/ipaddr/ipaddr.hpp>

int main() {
    using namespace polycpp::ipaddr;

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        if (!isValid(line)) {
            std::cout << line << "\tinvalid\n";
            continue;
        }
        auto v = parse(line);
        std::string rng = std::visit(
            [](const auto& a) { return a.range(); }, v);
        std::cout << line << '\t' << rng << '\n';
    }
    return 0;
}
