/* Copyright (c) 2012-2013 LevelDOWN contributors
 * See list at <https://github.com/rvagg/node-leveldown#contributing>
 * MIT +no-false-attribs License <https://github.com/rvagg/node-leveldown/blob/master/LICENSE>
 */

#include <cstdlib>
#include <node.h>
#include <node_buffer.h>
#include <iostream>
#include <pthread.h>

#include "database.h"

#include "leveldown.h"
#include "async.h"
#include "iterator_async.h"

using namespace std;
using namespace v8;
using namespace node;
using namespace leveldb;

/** NEXT WORKER **/

NextWorker::NextWorker (
    leveldown::Iterator* iterator
  , Persistent<Function> dataCallback
  , Persistent<Function> endCallback
) : AsyncWorker(database, dataCallback)
  , iterator(iterator)
  , endCallback(endCallback)
{};

NextWorker::~NextWorker () {}

void NextWorker::Execute () {
  ok = iterator->IteratorNext(key, value);
  if (!ok)
    status = iterator->IteratorStatus();
}

void NextWorker::HandleOKCallback () {
  Local<Value> returnKey;
  if (iterator->keyAsBuffer)
    returnKey = Local<Value>::New(Buffer::New((char*)key.data(), key.size())->handle_);
  else
    returnKey = String::New((char*)key.data(), key.size());
  Local<Value> returnValue;
  if (iterator->valueAsBuffer)
    returnValue = Local<Value>::New(Buffer::New((char*)value.data(), value.size())->handle_);
  else
    returnValue = String::New((char*)value.data(), value.size());
  if (ok) {
    Local<Value> argv[] = {
        Local<Value>::New(Null())
      , returnKey
      , returnValue
    };
    RunCallback(callback, argv, 3);
  } else {
    Local<Value> argv[0];
    RunCallback(endCallback, argv, 0);
  }
}

/** END WORKER **/

EndWorker::EndWorker (
    leveldown::Iterator* iterator
  , Persistent<Function> endCallback
) : AsyncWorker(database, endCallback)
  , iterator(iterator)
{};

EndWorker::~EndWorker () {}

void EndWorker::Execute () {
  iterator->IteratorEnd();
}
