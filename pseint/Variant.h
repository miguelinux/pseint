#ifndef VARIANT_H
#define VARIANT_H

#include <cassert>
#include <iostream>

#ifdef NDEBUG
#	define Assert(x) 
#else
#	define Assert(x) if(!(x)) { asm("int3"); asm("nop"); } else asm("nop")
#endif

/**
* Esta clase auxiliar me da el indice de un tipo del Variant4 (en qué posicion
* de la lista de tipos está, base 1). Lo hace mediante especialización parcial,
* por eso es clase y no funcion, ya que en c++98 no se puede hacer una 
* especialización parcial de una funcion o un método, solo de una clase.
**/
template<typename T, typename T1, typename T2, typename T3, typename T4>
struct Variant4Helper { static int ToInt(); };
template<typename T1, typename T2, typename T3, typename T4>
struct Variant4Helper<T1,T1,T2,T3,T4> { static int ToInt() { return 1; } };
template<typename T1, typename T2, typename T3, typename T4>
struct Variant4Helper<T2,T1,T2,T3,T4> { static int ToInt() { return 2; } };
template<typename T1, typename T2, typename T3, typename T4>
struct Variant4Helper<T3,T1,T2,T3,T4> { static int ToInt() { return 3; } };
template<typename T1, typename T2, typename T3, typename T4>
struct Variant4Helper<T4,T1,T2,T3,T4> { static int ToInt() { return 4; } };

template<typename T1, typename T2, typename T3, typename T4>
class Variant4 {
	int m_type;
	union aux_union_type {char a[sizeof(T1)],b[sizeof(T2)],c[sizeof(T3)],d[sizeof(T4)];};
	char m_data[sizeof(aux_union_type)];
	template<typename T> int TypeIndex() const { return Variant4Helper<T,T1,T2,T3,T4>::ToInt(); }
public:
	Variant4() : m_type(0) {}
	Variant4(const Variant4<T1,T2,T3,T4> &other) :m_type(0) { *this = other; }
	
	bool IsDefined() const { return m_type!=0; }
	template<typename T> bool Is() const { return m_type==TypeIndex<T>(); }
	template<typename T> void Set() { Assert(m_type==0); m_type = TypeIndex<T>(); new (m_data) T(); }
	template<typename T> void Set(const T &value) { Assert(m_type==0); m_type = TypeIndex<T>(); new (m_data) T(value); }
	template<typename T> void ReSet() { Variant4<T1,T2,T3,T4>::Clear(); Set<T>(); }
	template<typename T> void ForceSet(const T &value) { 
		if (Variant4<T1,T2,T3,T4>::template Is<T>()) Variant4<T1,T2,T3,T4>::template As<T>()=value;
		else { Variant4<T1,T2,T3,T4>::Clear(); Variant4<T1,T2,T3,T4>::template Set<T>(value); }
	}
	template<typename T> T &As() { Assert(m_type==TypeIndex<T>()); T *p = reinterpret_cast<T*>(m_data); return *p; }
	template<typename T> const T &As() const{ Assert(m_type==TypeIndex<T>()); const T* p =reinterpret_cast<const T*>(m_data); return *p; }
	void Clear() { 
		switch (m_type) {
		case 0: return;
		case 1: As<T1>().~T1(); break;
		case 2: As<T2>().~T2(); break;
		case 3: As<T3>().~T3(); break;
		case 4: As<T4>().~T4(); break;
		}
		m_type = 0;
	}
	Variant4<T1,T2,T3,T4> &operator=(const Variant4<T1,T2,T3,T4> &other) {
			 if (other.Is<T1>()) { ForceSet<T1>(other.As<T1>()); }
		else if (other.Is<T2>()) { ForceSet<T2>(other.As<T2>()); }
		else if (other.Is<T3>()) { ForceSet<T3>(other.As<T3>()); }
		else if (other.Is<T4>()) { ForceSet<T4>(other.As<T4>()); }
		else { Assert(!other.IsDefined()); Clear(); }
		return *this;
	}
	~Variant4() { Variant4<T1,T2,T3,T4>::Clear(); }
};

template<typename T1, typename T2, typename T3, typename T4>
std::ostream &operator<<(std::ostream &o, const Variant4<T1,T2,T3,T4> &v) {
	if (v.template Is<T1>()) return o << v.template As<T1>();
	if (v.template Is<T2>()) return o << v.template As<T2>();
	if (v.template Is<T3>()) return o << v.template As<T3>();
	if (v.template Is<T4>()) return o << v.template As<T4>();
	Assert(v.IsDefined()); return o;
}

#endif
