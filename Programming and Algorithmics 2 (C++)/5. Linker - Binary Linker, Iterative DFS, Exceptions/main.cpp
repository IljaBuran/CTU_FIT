#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <climits>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <optional>
#include <memory>
#include <stdexcept>
#include <set>
#include <map>
#include <queue>
#include <deque>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#endif /* __PROGTEST__ */

class CLinker
{
private:

    struct ObjFile;
    struct Symbol;
    struct FunctionBlob;
    
    std::vector<ObjFile>                          objectFiles_;
    std::unordered_map<std::string, Symbol>       symbolMap_;
    std::unordered_map<std::string, FunctionBlob> linkedFunctions_;
    std::vector<FunctionBlob*>                    linkOrder_;

public:
    
    CLinker()  noexcept  = default;
    ~CLinker() noexcept  = default;

    CLinker(const CLinker& other)            = delete;
    CLinker& operator=(const CLinker& other) = delete;

    CLinker& addFile (const std::string& fileName)
    {
        std::ifstream in(fileName, std::ios::binary);
        if (!in)
            throw std::runtime_error("Cannot read input file");

        Header header = ReadHeader(in);

        ObjFile objFile;
        objFile.exports.reserve(header.exportCount);
        for (uint32_t i = 0; i < header.exportCount; i++)
            objFile.exports.push_back(ReadExport(in));

        objFile.imports.reserve(header.importCount);
        for (uint32_t i = 0; i < header.importCount; i++)
            objFile.imports.push_back(ReadImport(in));

        objFile.code.resize(header.byteCount);
        if (header.byteCount > 0)
        {
            in.read(objFile.code.data(), header.byteCount);
            if (!in || static_cast<uint32_t>(in.gcount()) != header.byteCount)
                throw std::runtime_error("Cannot read input file");
        }

        objectFiles_.push_back(std::move(objFile));
        return *this;
    }

    void linkOutput (const std::string& fileName,
                     const std::string& entryPoint)
    {
        BuildSymbolTable();
        CollectFunctions(entryPoint);
        #ifdef SORT
        DEBUG_SORT();
        #endif
        Free();
        BindFunctionAddresses();
        Output(fileName);
    }

private:

    /* Helper structures */
    struct Header
    {
        uint32_t exportCount;
        uint32_t importCount;
        uint32_t byteCount;
    };
    
    struct ObjFile
    {
        struct Export
        {
            std::string name;
            uint32_t    offset;
        };

        struct Import
        {
            std::string           name;
            std::vector<uint32_t> positions;
        };

        std::vector<char>   code;
        std::vector<Export> exports;
        std::vector<Import> imports;
    };

    struct Symbol
    {
        size_t   fileIndex;
        uint32_t offset;
        uint32_t size;
    };

    struct FunctionBlob
    {
        struct Patch
        {
            uint32_t    offset;
            std::string symbolName;
        };
        
        std::string        name;
        std::vector<char>  code;
        std::vector<Patch> patches;
        uint32_t           address;
    };

    template<typename T>
    static inline T Read(std::istream& in)
    {
        T t;
        in.read(reinterpret_cast<char*>(&t), sizeof(t));
        if (!in)
            throw std::runtime_error("Cannot read input file");
        return t;
    }

    /* helpers for addFile() */
        static inline Header ReadHeader(std::ifstream& in)
        {
            /* read header from filestream */
            return 
            {
                Read<uint32_t>(in),
                Read<uint32_t>(in),
                Read<uint32_t>(in)
            };
        }

        static inline ObjFile::Export ReadExport(std::ifstream& in)
        {
            /* read number of characters in function name */
            uint8_t nameLength = Read<uint8_t>(in);
            
            /* read number of bytes into string */
            std::string fnName(nameLength, '\0');
            if (nameLength)
                in.read(fnName.data(), nameLength);
            
            /* read offset */
            uint32_t offset = Read<uint32_t>(in);

            return {std::move(fnName), offset};
        }

        static inline ObjFile::Import ReadImport(std::ifstream& in)
        {
            /* read number of characters in function name */
            uint8_t nameLength = Read<uint8_t>(in);

            /* read number of bytes into string */
            std::string fnName(nameLength, '\0');
            if (nameLength)
                in.read(fnName.data(), nameLength);
            
            /* read number function references */
            uint32_t referenceCount = Read<uint32_t>(in);
            
            /* read imports */
            ObjFile::Import import;
            import.name = std::move(fnName);
            import.positions.reserve(referenceCount);
            for (uint32_t i = 0; i < referenceCount; i++)
            {
                uint32_t position = Read<uint32_t>(in);;
                import.positions.push_back(position);
            }

            return import;
        }
    
    /* helpers for linkOutput() */
        void BuildSymbolTable()
        {
            if (!symbolMap_.empty())
                symbolMap_.clear();
            
            for (size_t fileIdx = 0; fileIdx < objectFiles_.size(); fileIdx++)
            {
                const ObjFile& objFile = objectFiles_[fileIdx];
                
                /* make a list of export indexes sorted by offset */
                std::vector<size_t> indexes(objFile.exports.size());
                for (size_t i = 0; i < indexes.size(); i++) indexes[i] = i;

                std::sort(indexes.begin(), indexes.end(), [&](size_t a, size_t b)
                {
                    return objFile.exports[a].offset < objFile.exports[b].offset;
                });

                /* calculate size of each export */
                for (size_t j = 0; j < indexes.size(); j++)
                {
                    const auto& _export = objFile.exports[indexes[j]];
                    uint32_t start = _export.offset;
                    uint32_t end = (j + 1 < indexes.size() ? 
                        objFile.exports[indexes[j + 1]].offset : objFile.code.size());
                    if (start >= end || end > objFile.code.size())
                        throw std::runtime_error("Invalid boundaries");
                    
                    Symbol symbol{fileIdx, start, end - start};
                    if (!symbolMap_.emplace(_export.name, symbol).second)
                        throw std::runtime_error("Duplicate symbol: " + _export.name);
                }
            }
        }

        void DFS(const std::string& entryPoint)
        {
            std::unordered_set<std::string> seen;
            std::stack<std::string> toProcess;
            toProcess.push(entryPoint);

            while (!toProcess.empty())
            {
                /* pop next symbol to process, if seen -> skip */
                std::string name = toProcess.top(); toProcess.pop();
                if (!seen.insert(name).second) continue;

                /* find symbol in global symbol table */
                auto it = symbolMap_.find(name);
                if (it == symbolMap_.end())
                    throw std::runtime_error("Undefined symbol: " + name);
                
                /* extract file index + code boundaries */
                const Symbol& symbol = it->second;
                const ObjFile& objFile = objectFiles_[symbol.fileIndex];

                /* build new linked function */
                FunctionBlob linkedFn;
                linkedFn.name = name;
                linkedFn.address = 0;
                linkedFn.code.assign(objFile.code.begin() + symbol.offset,
                                     objFile.code.begin() + symbol.offset + symbol.size);

                /* scan every import for patch locations */
                for (const auto& import : objFile.imports)
                    for (auto pos : import.positions)
                        if (pos >= symbol.offset && pos < symbol.offset + symbol.size)
                            linkedFn.patches.push_back(FunctionBlob::Patch{pos - symbol.offset, import.name});
                
                /* sort patches by offset */
                std::sort(linkedFn.patches.begin(), linkedFn.patches.end(), [](const auto& a, const auto& b)
                { 
                    return a.offset < b.offset; 
                });

                /* insert into linkedFunctions_ */
                auto insert = linkedFunctions_.emplace(name, std::move(linkedFn));
                linkOrder_.push_back(&insert.first->second);
                
                /* add referenced symbols for further processing */
                for (auto& patch : linkOrder_.back()->patches)
                    toProcess.push(patch.symbolName);
            }
        }

        void CollectFunctions(const std::string& entryPoint)
        {
            linkedFunctions_.clear(); linkOrder_.clear();
            DFS(entryPoint);
        }

        void Free()
        {
            objectFiles_.clear();
            objectFiles_.shrink_to_fit();
            symbolMap_.clear();
            symbolMap_.reserve(0);
        }

        void BindFunctionAddresses()
        {
            /* assign a addresses to functions */
            uint32_t currentOffset = 0;
            for (auto function : linkOrder_)
            {
                function->address = currentOffset;
                currentOffset += function->code.size();
            }
            
            /* apply each recorded patch */
            for (auto function : linkOrder_)
                for (auto& patch : function->patches)
                {
                    auto it = linkedFunctions_.find(patch.symbolName);
                    if (it == linkedFunctions_.end())
                        throw std::runtime_error("Undefined symbol: " + patch.symbolName);
                    uint32_t target = it->second.address;
                    if (patch.offset + sizeof(uint32_t) > function->code.size())
                        throw std::runtime_error("Invalid offset");
                    std::memcpy(function->code.data() + patch.offset, &target, sizeof(uint32_t));
                }
        }

        void Output(const std::string& outFile)
        {
            std::ofstream out(outFile, std::ios::binary);
            if (!out)
                throw std::runtime_error("Cannot write into output file");
            for (auto function : linkOrder_)
                out.write(function->code.data(), function->code.size());
            if (!out)
                throw std::runtime_error("Cannot write into output file");
            linkOrder_.clear();
        }

        #ifdef SORT
        void DEBUG_SORT()
        {
            if (linkOrder_.size() > 1)
            {
                std::sort(linkOrder_.begin() + 1, linkOrder_.end(), [](const FunctionBlob* a, const FunctionBlob* b)
                {
                    return a->name < b->name;
                });
            }
        }
        #endif
};

#ifndef __PROGTEST__
#ifdef TESTS
#include <filesystem>
namespace Tests
{
    static bool CompareFiles(const std::string& fileA, const std::string& fileB)
    {
        std::ifstream in1(fileA, std::ios::binary);
        std::ifstream in2(fileB, std::ios::binary);
        if (!in1 || !in2)
            throw std::runtime_error("Cannot open files for comparison");

        std::vector<char> buf1((std::istreambuf_iterator<char>(in1)),
                               std::istreambuf_iterator<char>());
        std::vector<char> buf2((std::istreambuf_iterator<char>(in2)),
                               std::istreambuf_iterator<char>());
        return buf1 == buf2;
    }
    
    void DeleteFile(const std::string& file)
    {
        try
        {
            std::filesystem::remove(file);
        }
        catch(const std::filesystem::filesystem_error& e)
        {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }
    }


    void Basic()
    {
        CLinker () . addFile ( "basic_tests/0in0.o" ) . linkOutput ( "basic_tests/0_out", "strlen" );
        assert(CompareFiles("basic_tests/0_out", "basic_tests/0ref"));
        DeleteFile("basic_tests/0_out");
    
        CLinker () . addFile ( "basic_tests/1in0.o" ) . linkOutput ( "basic_tests/1_out", "main" );
        assert(CompareFiles("basic_tests/1_out", "basic_tests/1ref"));
        DeleteFile("basic_tests/1_out");
        
        CLinker () . addFile ( "basic_tests/2in0.o" ) . addFile ( "basic_tests/2in1.o" ) . linkOutput ( "basic_tests/2_out", "main" );
        assert(CompareFiles("basic_tests/2_out", "basic_tests/2ref"));
        DeleteFile("basic_tests/2_out");
        
        CLinker () . addFile ( "basic_tests/3in0.o" ) . addFile ( "basic_tests/3in1.o" ) . linkOutput ( "basic_tests/3_out", "towersOfHanoi" );
        assert(CompareFiles("basic_tests/3_out", "basic_tests/3ref"));
        DeleteFile("basic_tests/3_out");
        
        try
        {
            CLinker () . addFile ( "basic_tests/4in0.o" ) . addFile ( "basic_tests/4in1.o" ) . linkOutput ( "basic_tests/4out", "unusedFunc" );
            assert ( "missing an exception" == nullptr );
        }
        catch ( const std::runtime_error & e )
        {
            #ifdef DEBUG
            std::cout << e.what() << '\n';
            #endif
            // e . what (): Undefined symbol qsort
            if (std::string(e.what()) != "Undefined symbol: qsort")
                throw std::runtime_error("Invalid fail reason\nExpected: Undefined symbol: qsort\nYou got: " + std::string(e.what()));
        }
        catch ( ... )
        {
            assert ( "invalid exception" == nullptr );
        }
        
        try
        {
            CLinker () . addFile ( "basic_tests/5in0.o" ) . linkOutput ( "basic_tests/5out", "main" );
            assert ( "missing an exception" == nullptr );
        }
        catch ( const std::runtime_error & e )
        {
            #ifdef DEBUG
            std::cout << e.what() << '\n';
            #endif
            // e . what (): Duplicate symbol: printf
            if (std::string(e.what()) != "Duplicate symbol: printf")
                throw std::runtime_error("Invalid fail reason\nExpected: Duplicate symbol: printf\nYou got: " + std::string(e.what()));
        }
        catch ( ... )
        {
            assert ( "invalid exception" == nullptr );
        }
        
        try
        {
            CLinker () . addFile ( "basic_tests/6in0.o" ) . linkOutput ( "basic_tests/6out", "strlen" );
            assert ( "missing an exception" == nullptr );
        }
        catch ( const std::runtime_error & e )
        {
            #ifdef DEBUG
            std::cout << e.what() << '\n';
            #endif
            // e . what (): Cannot read input file
            if (std::string(e.what()) != "Cannot read input file")
                throw std::runtime_error("Invalid fail reason\nExpected: Cannot read input file\nYou got: " + std::string(e.what()));
        }
        catch ( ... )
        {
            assert ( "invalid exception" == nullptr );
        }
        
        try
        {
            CLinker () . addFile ( "basic_tests/7in0.o" ) . linkOutput ( "basic_tests/7out", "strlen2" );
            assert ( "missing an exception" == nullptr );
        }
        catch ( const std::runtime_error & e )
        {
            #ifdef DEBUG
            std::cout << e.what() << '\n';
            #endif
            // e . what (): Undefined symbol strlen2
            if (std::string(e.what()) != "Undefined symbol: strlen2")
                throw std::runtime_error("Invalid fail reason\nExpected: Undefined symbol: strlen2\nYou got: " + std::string(e.what()));
        }
        catch ( ... )
        {
            assert ( "invalid exception" == nullptr );
        }
    }
    #ifdef EXTRA
    void Extra()
    {
        CLinker () . addFile ( "extra_tests/0010_0.o" ) . addFile ( "extra_tests/0010_1.o" ) . addFile ( "extra_tests/0010_2.o" ) . addFile ( "extra_tests/0010_3.o" ) . linkOutput ( "extra_tests/0010_out", "pdrolowjjgdwxiadj" );
        assert(CompareFiles("extra_tests/0010_out", "extra_tests/0010_ref"));
        DeleteFile("extra_tests/0010_out");
        
        CLinker () . addFile ( "extra_tests/0011_0.o" ) . addFile ( "extra_tests/0011_1.o" ) . linkOutput ( "extra_tests/0011_out", "yntvlhvtp" );
        assert(CompareFiles("extra_tests/0011_out", "extra_tests/0011_ref"));
        DeleteFile("extra_tests/0011_out");
        
        CLinker () . addFile ( "extra_tests/0012_0.o" ) . addFile ( "extra_tests/0012_1.o" ) . addFile ( "extra_tests/0012_2.o" ) . linkOutput ( "extra_tests/0012_out", "acnskqfuegem" );
        assert(CompareFiles("extra_tests/0012_out", "extra_tests/0012_ref"));
        DeleteFile("extra_tests/0012_out");
        
        CLinker () . addFile ( "extra_tests/0013_0.o" ) . addFile ( "extra_tests/0013_1.o" ) . addFile ( "extra_tests/0013_2.o" ) . linkOutput ( "extra_tests/0013_out", "yvjbkannhcusuktuhl" );
        assert(CompareFiles("extra_tests/0013_out", "extra_tests/0013_ref"));
        DeleteFile("extra_tests/0013_out");
        
        CLinker () . addFile ( "extra_tests/0014_0.o" ) . addFile ( "extra_tests/0014_1.o" ) . addFile ( "extra_tests/0014_2.o" ) . linkOutput ( "extra_tests/0014_out", "adqcwiahautvfi" );
        assert(CompareFiles("extra_tests/0014_out", "extra_tests/0014_ref"));
        DeleteFile("extra_tests/0014_out");
    }
    #endif
};
#endif

int main()
{
    #ifdef TESTS
    Tests::Basic();
    

    #ifdef EXTRA
    Tests::Extra();
    #endif 
    #endif 
    
    return EXIT_SUCCESS;
}
#endif