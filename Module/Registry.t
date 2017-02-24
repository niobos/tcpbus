#pragma once

#include <vector>
#include <map>
#include <stdexcept>

template< class T_id, class T_data, int unique=0 >
class Registry {
/* unique can be used to make 2 registrar's with the same types */

	/* Singleton pattern */
public:
	static Registry& get_instance() {
		static Registry single_instance;
		return single_instance;
	}
	/* returns a reference to the only existing instance of this class */
private:
	Registry() {};
	/* Private constructor */

public:
	void add( const T_id& id, const T_data& data ) throw(std::runtime_error) {
	/* Register data under id */
		if( m_register.find(id) != m_register.end() ) {
			throw std::runtime_error("duplicate key" + id);
		}
		m_register[id] = data;
	}

	std::vector< T_id > get_list() const throw() {
	/* Returns the list of ID's currently registered */
		std::vector< T_id > list;
		for( auto i = m_register.begin(); i != m_register.end(); i++ ) {
			list.push_back( i->first );
		}
		return list;
	}

	T_data get( const T_id& id ) const throw(std::runtime_error) {
	/* Lookup the data for ID */
		auto i = m_register.find( id );
		if( i == m_register.end() ) throw std::runtime_error("Registry: couldn't find " + id);
		return i->second;
	}

private:
	std::map< T_id, T_data > m_register;
};
