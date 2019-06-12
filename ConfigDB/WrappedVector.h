#ifndef DEF_WRAPPED_VECTOR
#define DEF_WRAPPED_VECTOR

#include <vector>
#include <algorithm>

template<class T>
class WrappedVector
{
private:
	std::vector<T> m_vector;
public:
	WrappedVector() {}
	WrappedVector(int n) : m_vector(n) {}
	void clear(){ m_vector.clear(); }
	void push_back(T& x){ m_vector.push_back(x); }
	void pop_back(T& x){ m_vector.pop_back(x); }
	int size() const { return m_vector.size(); }
	int capacity() const { return m_vector.capacity(); }
	T& getAt(int n){ return m_vector[n]; }
	T& operator[](int n){ return m_vector[n]; }
	const T& operator[](int n) const { return m_vector[n]; }
	bool erase(T& x){
		std::vector<T>::iterator itr = std::find(m_vector.begin(), m_vector.end(), x);
		if(itr != m_vector.end()){
			m_vector.erase(itr);
			return true;
		}else{
			return false;
		}
	}
	bool isExist(T& x) const {
		std::vector<T>::const_iterator itr = std::find(m_vector.begin(), m_vector.end(), x);
		return itr != m_vector.end();
	}
};

#endif
