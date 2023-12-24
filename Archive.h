#ifndef LABWORK6_ARCHIVE_H
#define LABWORK6_ARCHIVE_H

#include "Parser.h"
#include <cstdint>

class Archive {
public:
    static void CreateArchive(const Arguments& parse_arguments);
    static void AppendFiles(const Arguments& parse_arguments);
    static void ShowList(const Arguments& parse_arguments);
    static void ConcatenateArchives(const Arguments& parse_arguments);;
    static void Extract(const Arguments& parse_arguments);
    static void Delete(const Arguments& parse_arguments);
};

void AnaliseArgs(const Arguments& parse_args);

#endif //LABWORK6_ARCHIVE_H
