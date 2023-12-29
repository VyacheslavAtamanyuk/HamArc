#include "Parser.h"
#include "Archive.h"

int main(int argc, char** argv) {
    Arguments new_arguments = Parse(argc, argv);

    AnalyzeArgs(new_arguments);
    return 0;
}
