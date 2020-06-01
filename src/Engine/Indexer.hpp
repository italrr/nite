#ifndef NITE_ASSET_INDEXER_HPP
    #define NITE_ASSET_INDEXER_HPP

    #include "Tools/Tools.hpp"

    namespace nite {

        struct IndexedFile {
            String path;
            String hash;
            UInt32 size;
            UInt64 lastRead;
            Vector<String> tags;
            IndexedFile();
            void refresh(const String &path);
            void refresh();
            void autotag();
            bool isIt(const String &tag);
        };

        struct Indexer {
            bool listening;
            Dict<String, IndexedFile> indexed;
            Indexer();
            bool indexDir(const String &path);
            nite::IndexedFile *indexFile(const String &file);
            nite::IndexedFile *indexGhostFile(const String &path, const String &hash);
            nite::IndexedFile *get(const String &hash);
        };

        nite::Indexer *getIndexer();
    }


#endif