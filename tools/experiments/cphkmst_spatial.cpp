// Query benchmark: the same rounded-Euclidean k-nearest distances via scan/KD.
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>
extern "C" {
#include "machdefs.h"
#include "util.h"
#include "kdtree.h"
}
int main() {
    using Clock=std::chrono::steady_clock;
    auto seconds=[](Clock::time_point t) {return std::chrono::duration<double>(Clock::now()-t).count();};
    try {
        for (int n : {100,1000,10000}) for (int k : {8,20}) {
            CCdatagroup dat; CCutil_init_datagroup(&dat);
            CCutil_dat_setnorm(&dat,CC_EUCLIDEAN);
            dat.x=CC_SAFE_MALLOC(n,double); dat.y=CC_SAFE_MALLOC(n,double);
            if(!dat.x||!dat.y) throw std::bad_alloc();
            std::mt19937 random(11923);
            for(int u=0;u<n;++u) {dat.x[u]=random()%1000000; dat.y[u]=random()%1000000;}
            CCrandstate rng; CCutil_sprand(1,&rng);
            std::vector<int> expected(n*k),actual(n*k),neighbors(k),distances;
            auto t=Clock::now();
            for(int u=0;u<n;++u) {
                distances.clear(); distances.reserve(n-1);
                for(int v=0;v<n;++v) if(u!=v) distances.push_back(CCutil_dat_edgelen(u,v,&dat));
                std::nth_element(distances.begin(),distances.begin()+k,distances.end());
                std::sort(distances.begin(),distances.begin()+k);
                std::copy_n(distances.begin(),k,expected.begin()+u*k);
            }
            double scan=seconds(t);
            CCkdtree tree;
            t=Clock::now();
            if(CCkdtree_build(&tree,n,&dat,nullptr,&rng)) throw std::runtime_error("KD build failed");
            for(int u=0;u<n;++u) {
                if(CCkdtree_node_k_nearest(&tree,n,u,k,&dat,nullptr,neighbors.data(),&rng))
                    throw std::runtime_error("KD query failed");
                for(int i=0;i<k;++i) {
                    if(neighbors[i]<0||neighbors[i]>=n||neighbors[i]==u) throw std::runtime_error("bad KD neighbor");
                    actual[u*k+i]=CCutil_dat_edgelen(u,neighbors[i],&dat);
                }
                std::sort(actual.begin()+u*k,actual.begin()+(u+1)*k);
            }
            double kd=seconds(t);
            CCkdtree_free(&tree); CCutil_freedatagroup(&dat);
            if(actual!=expected) throw std::runtime_error("KD differs from exhaustive nearest distances");
            std::cout<<"{\"n\":"<<n<<",\"k\":"<<k<<",\"queries_verified\":"<<n
                <<",\"scan_seconds\":"<<scan<<",\"kd_seconds\":"<<kd<<"}\n";
        }
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
