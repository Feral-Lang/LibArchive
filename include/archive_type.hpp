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

#ifndef FERAL_LIB_ARCHIVE_TYPE_HPP
#define FERAL_LIB_ARCHIVE_TYPE_HPP

#include <archive_entry.h>
#include <archive.h>
#include <feral/VM/VM.hpp>

enum open_mode_t
{
	OM_READ,
	OM_WRITE,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////// ARCHIVE class //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class var_archive_t : public var_base_t
{
	archive * m_val;
	open_mode_t m_mode;
	bool m_owner;
public:
	var_archive_t( archive * const val, const int & mode, const size_t & src_id, const size_t & idx, const bool owner = true );
	~var_archive_t();

	var_base_t * copy( const size_t & src_id, const size_t & idx );
	void set( var_base_t * from );

	archive * const get();
	inline const open_mode_t & mode() const { return m_mode; }
};
#define ARCHIVE( x ) static_cast< var_archive_t * >( x )

class var_archive_entry_t : public var_base_t
{
	archive_entry * m_val;
	bool m_owner;
public:
	var_archive_entry_t( archive_entry * const val, const size_t & src_id, const size_t & idx, const bool owner = true );
	~var_archive_entry_t();

	var_base_t * copy( const size_t & src_id, const size_t & idx );
	void set( var_base_t * from );

	archive_entry * const get();
};
#define ARCHIVE_ENTRY( x ) static_cast< var_archive_entry_t * >( x )

#endif // FERAL_LIB_ARCHIVE_TYPE_HPP