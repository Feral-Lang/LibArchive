/*
	MIT License

	Copyright (c) 2020 Feral Language repositories

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so.
*/

#include "../include/archive_type.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////// Archive class ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

var_archive_t::var_archive_t( archive * const val, const int & mode, const size_t & src_id, const size_t & idx, const bool owner )
	: var_base_t( type_id< var_archive_t >(), src_id, idx, false, false ), m_val( val ), m_mode( ( open_mode_t )mode ), m_owner( owner )
{}
var_archive_t::~var_archive_t()
{
	if( m_owner && m_val ) {
		if( m_mode == OM_READ ) archive_read_free( m_val );
		else if( m_mode == OM_WRITE ) archive_write_free( m_val );
	}
}

var_base_t * var_archive_t::copy( const size_t & src_id, const size_t & idx )
{
	return new var_archive_t( m_val, m_mode, src_id, idx, false );
}

void var_archive_t::set( var_base_t * from )
{
	if( m_owner && m_val ) archive_read_free( m_val );
	m_owner = false;
	m_mode = ARCHIVE( from )->m_mode;
	m_val = ARCHIVE( from )->m_val;
}

archive * const var_archive_t::get() { return m_val; }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// Archive Entry class /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

var_archive_entry_t::var_archive_entry_t( archive_entry * const val, const size_t & src_id, const size_t & idx, const bool owner )
	: var_base_t( type_id< var_archive_entry_t >(), src_id, idx, false, false ), m_val( val ), m_owner( owner )
{}
var_archive_entry_t::~var_archive_entry_t()
{
	archive_entry_free( m_val );
}

var_base_t * var_archive_entry_t::copy( const size_t & src_id, const size_t & idx )
{
	return new var_archive_entry_t( m_val, src_id, idx, false );
}

void var_archive_entry_t::set( var_base_t * from )
{
	if( m_owner && m_val ) archive_entry_free( m_val );
	m_owner = false;
	m_val = ARCHIVE_ENTRY( from )->m_val;
}

archive_entry * const var_archive_entry_t::get() { return m_val; }