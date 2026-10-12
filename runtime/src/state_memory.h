

#pragma once
#include "savestate.h"
#include "platform/host.h"
#include <algorithm>
#include <atomic>
#include <mutex>
#include <thread>
#include <unordered_map>
namespace ss {
inline constexpr uint32_t kMemoryChunk=0x10000;
constexpr size_t kChunkBatch=32;
struct MemoryRegion {uint32_t base,size;};
using MemoryChunks=std::unordered_map<uint32_t,const uint8_t*>;
inline bool memory_chunk_zero(const uint8_t* p) {
    for(uint32_t i=0;i<kMemoryChunk;i+=8){uint64_t word;memcpy(&word,p+i,sizeof word);if(word)return false;}
    return true;
}
inline unsigned chunk_workers(size_t chunks) {
    if(chunks<128)return 1;
    unsigned hw=std::thread::hardware_concurrency();
    return hw<2?1:(hw<16?hw:16);
}
template<class Pointer,class Touched>
struct Residency {
    Pointer& ptr;
    Touched& touched;
    uint32_t base=0;
    size_t pages=0;
    std::vector<uint8_t> page;
    void region(uint32_t region_base,size_t bytes) {
        base=0,pages=0,page.clear();
        if(!host::memory_resident_supported())return;
        pages=(bytes+host::page_size()-1)/host::page_size();
        page.assign(pages,1);
        if(host::memory_resident(ptr(region_base),bytes,page.data()))return;
        for(auto& v:page)v&=1u;
        base=region_base;
    }
    bool check(const uint8_t* p,size_t bytes) const {
        if(!pages)return touched(p,bytes);
        size_t off=(uintptr_t)p-(uintptr_t)base;
        size_t first=off/host::page_size(),last=(off+bytes-1)/host::page_size();
        if(first>=pages||last>=pages)return touched(p,bytes);
        for(size_t i=first;i<=last;++i)if(page[i])return true;
        return false;
    }
};
template<class Pointer,class Touched>
void capture_regions(Writer& w,const std::vector<MemoryRegion>& regions,Pointer ptr,Touched touched) {
    struct Run { std::vector<uint8_t> data; };
    Residency<Pointer,Touched> res{ptr,touched};
    w.u32(uint32_t(regions.size()));
    for(const auto& g:regions) {
        w.u32(g.base);w.u32(g.size);size_t count_at=w.b.size();w.u32(0);
        std::atomic<uint32_t> present{0};
        std::mutex mu;
        res.region(g.base,g.size);
        const size_t chunks=size_t(g.size/kMemoryChunk);
        unsigned n=chunk_workers(chunks);
        const auto scan=[&](size_t first,size_t last,Run& r) {
            for(size_t i=first;i<last;++i) {
                const uint8_t* p=ptr(g.base+uint32_t(i*kMemoryChunk));
                if(!res.check(p,kMemoryChunk))continue;
                if(memory_chunk_zero(p))continue;
                size_t pos=r.data.size();
                r.data.resize(pos+4+kMemoryChunk);
                uint32_t index=uint32_t(i);
                memcpy(r.data.data()+pos,&index,4);
                memcpy(r.data.data()+pos+4,p,kMemoryChunk);
                ++present;
            }
        };
        const auto flush=[&](Run& r) {
            if(r.data.empty())return;
            std::lock_guard<std::mutex> lk(mu);
            w.bytes(r.data.data(),r.data.size());
            r.data.clear();
        };
        if(n<=1) {
            Run r;
            for(size_t i=0;i<chunks;i+=kChunkBatch) {
                scan(i,std::min(i+kChunkBatch,chunks),r);
                flush(r);
            }
        } else {
            std::atomic<size_t> next{0};
            std::vector<std::thread> pool;
            pool.reserve(n-1);
            const auto walk=[&]() {
                Run r;
                for(size_t i;(i=next.fetch_add(kChunkBatch,std::memory_order_relaxed))<chunks;) {
                    scan(i,std::min(i+kChunkBatch,chunks),r);
                    flush(r);
                }
                flush(r);
            };
            for(unsigned t=1;t<n;++t)pool.emplace_back(walk);
            walk();
            for(auto& t:pool)t.join();
        }
        uint32_t count=present.load(std::memory_order_relaxed);
        memcpy(w.b.data()+count_at,&count,sizeof count);
    }
}
inline bool read_regions(Reader& r,std::vector<MemoryRegion>& regions,MemoryChunks& chunks) {
    uint32_t count=r.u32();if(count>64)return false;
    for(uint32_t i=0;i<count;++i){
        MemoryRegion g{r.u32(),r.u32()};
        if(!r.ok||(g.base&(kMemoryChunk-1))||(g.size&(kMemoryChunk-1))||uint64_t(g.base)+g.size>0x100000000ull)return false;
        for(const auto& previous:regions)if(uint64_t(g.base)<uint64_t(previous.base)+previous.size&&uint64_t(previous.base)<uint64_t(g.base)+g.size)return false;
        regions.push_back(g);uint32_t present=r.u32();if(present>g.size/kMemoryChunk)return false;
        for(uint32_t j=0;j<present;++j){uint32_t index=r.u32();
            if(index>=g.size/kMemoryChunk||!chunks.emplace(g.base+index*kMemoryChunk,r.p).second||!r.bytes(nullptr,kMemoryChunk))return false;
        }
    }
    return r.ok&&r.at_end();
}
template<class Pointer,class Touched>
void restore_regions(const std::vector<MemoryRegion>& regions,const MemoryChunks& chunks,Pointer ptr,Touched touched) {
    Residency<Pointer,Touched> res{ptr,touched};
    for(const auto& g:regions){
        size_t nchunks=size_t(g.size/kMemoryChunk);
        res.region(g.base,g.size);
        unsigned n=chunk_workers(nchunks);
        const auto fill=[&](size_t first,size_t last) {
            for(size_t i=first;i<last;++i){
                uint32_t address=g.base+uint32_t(i*kMemoryChunk);uint8_t* p=ptr(address);const auto found=chunks.find(address);
                if(found!=chunks.end()){memcpy(p,found->second,kMemoryChunk);continue;}
                if(res.check(p,kMemoryChunk)&&!memory_chunk_zero(p))memset(p,0,kMemoryChunk);
            }
        };
        if(n<=1){fill(0,nchunks);continue;}
        std::atomic<size_t> next{0};
        std::vector<std::thread> pool;
        pool.reserve(n-1);
        const auto walk=[&]() {
            for(size_t i;(i=next.fetch_add(kChunkBatch,std::memory_order_relaxed))<nchunks;)
                fill(i,std::min(i+kChunkBatch,nchunks));
        };
        for(unsigned t=1;t<n;++t)pool.emplace_back(walk);
        walk();
        for(auto& t:pool)t.join();
    }
}
}
