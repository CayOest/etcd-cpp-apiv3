#include "v3/include/AsyncTxnResponse.hpp"
#include "v3/include/AsyncRangeResponse.hpp"

using etcdserverpb::ResponseOp;


etcdv3::AsyncTxnResponse::AsyncTxnResponse(const etcdv3::AsyncTxnResponse& other) 
{
  error_code = other.error_code;
  error_message = other.error_message;
  index = other.index;
  action = other.action;
  values = other.values;
  prev_value.set_key(other.prev_value.key());
  prev_value.set_value(other.prev_value.value());
  prev_value.set_create_revision(other.prev_value.create_revision());
  prev_value.set_mod_revision(other.prev_value.mod_revision());

}

etcdv3::AsyncTxnResponse& etcdv3::AsyncTxnResponse::operator=(const etcdv3::AsyncTxnResponse& other) 
{
  error_code = other.error_code;
  error_message = other.error_message;
  index = other.index;
  action = other.action;
  values = other.values;
  prev_value.set_key(other.prev_value.key());
  prev_value.set_value(other.prev_value.value());
  prev_value.set_create_revision(other.prev_value.create_revision());
  prev_value.set_mod_revision(other.prev_value.mod_revision());
  return *this;
}

etcdv3::AsyncTxnResponse& etcdv3::AsyncTxnResponse::ParseResponse()
{
  if(!status.ok())
  {
    error_code = status.error_code();
    error_message = status.error_message();
  }
  else
  {
    std::vector<mvccpb::KeyValue> range_kvs;
    for(int index=0; index < reply.responses_size(); index++)
    {
      auto resp = reply.responses(index);
      if(ResponseOp::ResponseCase::kResponseRange == resp.response_case())
      {
        AsyncRangeResponse response;
        response.reply = resp.response_range();
        auto v3resp = response.ParseResponse();
     
        error_code = v3resp.error_code;
        error_message = v3resp.error_message;

        if(!v3resp.values.empty())
        {
          range_kvs.insert(range_kvs.end(), v3resp.values.begin(), v3resp.values.end());
        }
      }
    }

    if(!reply.succeeded())
    { 
      if(action == "create")
      {
        error_code=105;
        error_message="Key already exists";
      }
      else if(action == "compareAndSwap")
      {
        error_code=101;
        error_message="Compare failed";
      }
    }

    //find previous value of key do this for all actions except get
    //retain only the last value gotten as the final value.
    if(action != "get" && range_kvs.size() > 1)
    {
      prev_value = range_kvs.front();
      values.push_back(range_kvs.back());    
    }
    else
    {
      values = range_kvs;
    }
  }
  return *this;
}