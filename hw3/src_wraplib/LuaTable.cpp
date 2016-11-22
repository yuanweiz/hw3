#include "LuaConfig.h"
using namespace std;
class LuaTable::Impl:Noncopyable{
    public:
        template <typename T>
            void push_back (const T& val){
                elements_.push_back(make_shared<T> (val));
            }
        template <typename T>
            T& get(int idx){
                return *static_cast<T*>(elements_[idx].get());
            }
        size_t size(){return elements_.size();}
    private:
        vector<shared_ptr<void>> elements_;
};

size_t LuaTable::size(){
    return pimpl->size();
}
LuaTable::LuaTable():pimpl(new Impl) {
}

//now compiler can find the defination of ~unique_ptr<LuaTable::Impl>
LuaTable::~LuaTable(){}

LuaTable & LuaTable::operator=(LuaTable&&rhs){
    pimpl = std::move(rhs.pimpl);
    return *this;
}

template <typename T>
void LuaTable::push_back(const T& val){
    pimpl->push_back<T>(val);
}
template <typename T>
T& LuaTable::get(int idx){
    return pimpl->get<T>(idx);
}

//explicit instantiate, currently not supporting LuaTable itself
template void LuaTable::push_back(const int&);
template void LuaTable::push_back(const double&);
template void LuaTable::push_back(const bool&);
template void LuaTable::push_back(const string&);
template double & LuaTable::get (int);
template bool & LuaTable::get (int);
template int & LuaTable::get (int);
template string & LuaTable::get (int);

LuaTable::LuaTable( LuaTable&& rhs)
    : pimpl(std::move(rhs.pimpl))
{
}
