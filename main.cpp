#include "Parser.h"
#include "Requests.h"

int main(int argc, char** argv) {
    Arguments new_arguments = Parse(argc, argv);

    Requests my_requests(new_arguments.archive_name, new_arguments.hamming_block);
    my_requests.AnalyzeArgs(new_arguments);
    return 0;
}
