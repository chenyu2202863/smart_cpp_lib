#include "db.hpp"

#include <cstdint>
#include <cassert>
#include <sstream>

#include "exception.hpp"
#include "variant_traits.hpp"

#import "../../db/ado/msado15.dll" no_namespace rename("EOF","adoEOF")


namespace async { namespace proto { namespace detail {


	void test_result(HRESULT hr)
	{
		if( hr != S_OK )
		{
			assert(0);
			throw _com_error(hr);
		}
	}

	struct recordset_t::impl
	{
		_RecordsetPtr rds_impl_;

		impl()
		{
			test_result(rds_impl_.CreateInstance(__uuidof(Recordset)));
			rds_impl_->CursorLocation = adUseClient;
			rds_impl_->CursorType = adOpenStatic;
		}
	};


	recordset_t::recordset_t()
		: impl_(new impl)
	{

	}

	recordset_t::~recordset_t()
	{

	}

	bool recordset_t::is_empty() const
	{
		assert(impl_->rds_impl_);
		return impl_->rds_impl_->GetRecordCount() == 0;
	}

	bool recordset_t::is_eof() const
	{
		assert(impl_->rds_impl_);
		return impl_->rds_impl_->GetadoEOF() == -1;
	}

	bool recordset_t::next()
	{
		assert(impl_->rds_impl_);
		return impl_->rds_impl_->MoveNext() == S_OK;
	}

	std::uint32_t recordset_t::get_records_count() const
	{
		assert(impl_->rds_impl_);
		return impl_->rds_impl_->GetRecordCount();
	}

	void recordset_t::set_connection(const connection_t &connection)
	{

	}

	void recordset_t::execute()
	{

	}

	std::uint32_t recordset_t::get_field_count() const
	{
		try
		{
			FieldsPtr fields_ptr = impl_->rds_impl_->GetFields();
			if( fields_ptr != nullptr ) 
				return fields_ptr->GetCount();
			else
				return 0;
		}
		catch(_com_error &e)
		{
			throw db_exception((const char *)e.Description());
		}
	}

	std::string recordset_t::get_field_name(std::uint32_t index) const
	{
		try
		{
			FieldsPtr fields_ptr = impl_->rds_impl_->GetFields();
			if( fields_ptr ) 
			{
				FieldPtr field_ptr = fields_ptr->GetItem((long)index);
				if( field_ptr ) 
				{
					return std::string(field_ptr->GetName());
				}
			}

			assert(0);
			return std::string();
		}
		catch(_com_error &e)
		{
			throw db_exception((const char *)e.Description());
		}
	}

	_variant_t recordset_t::get_field_value(const std::string &filed_name) const
	{
		assert(impl_->rds_impl_);

		try
		{
			return impl_->rds_impl_->GetCollect(filed_name.c_str());
		}
		catch(_com_error &e)
		{
			throw db_exception((const char *)e.Description());
		}
	}

	_variant_t recordset_t::get_field_value(std::uint32_t index) const
	{
		assert(impl_->rds_impl_);

		try
		{
			return impl_->rds_impl_->GetCollect((long)index);
		}
		catch(_com_error &e)
		{
			throw db_exception((const char *)e.Description());
		}
	}

	
	

	struct connection_t::impl
	{
		_ConnectionPtr connection_;
		error_handler_t handler_;

		impl(const error_handler_t &handler)
			: handler_(handler)
		{
		}

		~impl()
		{
			
		}

		impl(impl &&rhs)
			: connection_(std::move(rhs.connection_))
			, handler_(std::move(rhs.handler_))
		{}

		bool start(const std::string &connection_cmd)
		{
			try 
			{
				test_result(connection_.CreateInstance("ADODB.Connection"));
				test_result(connection_->Open(connection_cmd.c_str(), "", "", adModeUnknown));
			} 
			catch(_com_error &e) 
			{
				handler_((const char *)e.Description());
				
				return false;
			}

			return true;
		}

		bool stop()
		{
			try 
			{
				assert(connection_);
				connection_->Close();
			} 
			catch(_com_error &e) 
			{
				handler_((const char *)e.Description());

				return false;
			}

			return true;
		}
	};


	connection_t::connection_t(const error_handler_t &handler)
		: impl_(new impl(handler))
	{

	}

	connection_t::~connection_t()
	{

	}

	connection_t::connection_t(connection_t &&rhs)
		: impl_(std::move(rhs.impl_))
	{

	}

	bool connection_t::start(const std::string &connection_cmd)
	{
		return impl_->start(connection_cmd); 
	}

	bool connection_t::stop()
	{
		return impl_->stop();
	}

	std::uint32_t connection_t::execute(const std::string &sql)
	{
		try 
		{
			_CommandPtr command;
			test_result(command.CreateInstance(__uuidof(Command)));

			command->PutCommandText(sql.c_str());
			command->PutCommandType(adCmdText);
			command->ActiveConnection = impl_->connection_; 

			command->Execute(0, 0, adCmdText);
			
			auto rds = get_recordset("select @@IDENTITY");
			return rds->get_field_value(0);
		} 
		catch(const _com_error& e) 
		{
			std::ostringstream os;
			os << "'" << sql << "'" << std::endl
				<< "because: " << (const char *)e.Description();
			throw db_exception(os.str().c_str());
		}
	}

	std::uint32_t connection_t::execute(const command_ptr &command)
	{
		try
		{
			command->set_connection(*this);
			command->execute();

			auto rds = get_recordset("select @@IDENTITY");
			return rds->get_field_value(0);
		}
		catch(const _com_error &e)
		{
			std::ostringstream os;
			os << "'" << "command execute failed" << "'" << std::endl
				<< "because: " << ( const char * )e.Description();

			throw db_exception(os.str().c_str());
		}
	}

	recordset_ptr connection_t::get_recordset(const std::string &sql)
	{
		try 
		{
 			_CommandPtr command;
			test_result(command.CreateInstance(__uuidof(Command)));
			
			command->PutCommandText(sql.c_str());
			command->PutCommandType(adCmdText);
			command->ActiveConnection = impl_->connection_; 


			recordset_ptr rds = std::make_shared<recordset_t>();
			test_result(rds->impl_->rds_impl_->Open(static_cast<IDispatch *>(command), vtMissing, adOpenStatic, adLockBatchOptimistic, adOptionUnspecified));

			return rds;
		} 
		catch(const _com_error& e) 
		{
			std::ostringstream os;
			os << "'" << sql << "'" << std::endl
				<< "because: " << (const char *)e.Description();
			throw db_exception(os.str().c_str());
		}
	}

	recordset_ptr connection_t::get_recordset(const command_ptr &command)
	{
		try
		{
			command->set_connection(*this);
			return command->get_recordset();
		}
		catch( const _com_error &e )
		{
			std::ostringstream os;
			os << "'" << "command execute failed" << "'" << std::endl
				<< "because: " << (const char *)e.Description();

			throw db_exception(os.str().c_str());
		}
	}

	void connection_t::update_batch()
	{
		try
		{
			recordset_ptr rds = std::make_shared<recordset_t>();

			test_result(rds->impl_->rds_impl_->Open("SELECT * from blockchannel;", impl_->connection_.GetInterfacePtr(), adOpenKeyset, adLockBatchOptimistic, adCmdText));

			assert(rds->impl_->rds_impl_->Supports(adAddNew) && 
				   rds->impl_->rds_impl_->Supports(adUpdateBatch)); 

			int i = 0;
			while(!rds->impl_->rds_impl_->adoEOF)
			{
				std::ostringstream os;
				os << "test_" << i++;

				rds->impl_->rds_impl_->PutCollect("channel", os.str().c_str());
				rds->impl_->rds_impl_->MoveNext();
			}

			rds->impl_->rds_impl_->UpdateBatch(adAffectAll);
		}
		catch(_com_error &e)
		{
			std::ostringstream os;
			os << "'update_batch'" << std::endl
				<< "because: " << ( const char * )e.Description();
			throw db_exception(os.str().c_str());
		}
	}

	struct parameters_t::impl
	{
		ParametersPtr params_;

		impl(const ParametersPtr &params)
			: params_(params)
		{}
	};

	struct command_t::impl
	{
		_CommandPtr command_;

		impl(const std::string &sql)
		{
			test_result(command_.CreateInstance("ADODB.Command"));
			command_->CommandText = sql.c_str();
			command_->CommandType = adCmdText;
			command_->Prepared = TRUE;
		}
		~impl()
		{
			
		}

		void set_connection(const connection_t & connection)
		{
			command_->ActiveConnection = connection.impl_->connection_;
		}

		recordset_ptr get_recordset()
		{
			recordset_ptr rds = std::make_shared<recordset_t>();
			auto rds_ptr = command_->Execute(0, 0, adCmdText);
			rds->impl_->rds_impl_ = rds_ptr;
			return rds;
		}

		void execute()
		{
			command_->Execute(0, 0, adCmdText);
		}
	};


	command_t::command_t(const std::string &sql)
		: impl_(std::make_unique<impl>( sql ))
	{}

	command_t::~command_t()
	{}

	void command_t::set_connection(const connection_t &connection)
	{
		impl_->set_connection(connection);
	}

	recordset_ptr command_t::get_recordset()
	{
		return impl_->get_recordset();
	}

	void command_t::execute()
	{
		impl_->execute();
	}

	void command_t::_append_impl(const std::string &name, const _variant_t &val, DataTypeEnum type, std::uint32_t val_size)
	{
		try
		{
			auto param = impl_->command_->CreateParameter(_bstr_t(name.c_str()), (::DataTypeEnum)variant_type(val), adParamInput, val_size, val);
			impl_->command_->GetParameters()->Append(param);
		}
		catch(const _com_error &e)
		{
			std::ostringstream os;
			os << "' command append parameter error'" << std::endl
				<< "because: " << ( const char * )e.Description();
			throw db_exception(os.str().c_str());
		}
	}

	void command_t::append(long index, const _variant_t &v)
	{
		
		try
		{
			auto param = impl_->command_->CreateParameter(_bstr_t(index), (::DataTypeEnum)variant_type(v), adParamInput, -1, v);
			impl_->command_->GetParameters()->Append(param);
		}
		catch(const _com_error &e)
		{
			std::ostringstream os;
			os << "' command append parameter error'" << std::endl
				<< "because: " << ( const char * )e.Description();
			throw db_exception(os.str().c_str());
		}
	}


	parameters_t::parameters_t(const command_ptr &command)
		: impl_(std::make_unique<impl>(command->impl_->command_->GetParameters()))
	{}

	parameters_t::~parameters_t()
	{}
	
}}}
