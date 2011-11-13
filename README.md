# serstl - STL containers serialization to standard streams

## Purpose

This is a small hack to serialize native STL construct.
It is supposed to let you write and read them from a standard C++ stream, given that you know the original type.

Typical usage is, dumping a few objects to a stream, and then retieving them back.

    ofstream temp;
    serstl::put(mymap);
    serstl::put(myvector);
    serstl::put(myint);
    ...
    if (serstl::get(&mymap).isError()) barp();
    if (serstl::get(&myvector).isError()) barp();
    Error err = serstl::get(&myint);
    if (err.isError()) {std::cerr << "Error in stream: " << err.what() << std::endl;}

It's a header only library.

## Technical details

### Error Handling

In a similar fashion to Google Go (golang), and to support environments where exceptions are forbiddened, the
code never throws an exception explicitly. Instead, the serstl::Error class is returned. This is a simple class
as wide as a pointer, and one would typically test if the error is not nil with `err.isError()`. If it is indeed
a non-nil error, you can get a textual representation of the error with `err.what()`, similar to `std::exception`.

All `get` and `put` operations tries to unread every illegal input byte from the stream. So in order to get the
location of the error, one is expected to count how many bytes where read from the stream.

We are lenient on purpose. That is, we will allow deserializing a serialized `vector<int>` into `list<int>`, or
`pair<int,int>` to `vector<int>`. As far as we concern `map<int,string>` is a `vector<pair<int,string> >`.

### Interopability

The binary output is a legal [bencode][http://en.wikipedia.org/wiki/Bencode] stream. Since bencode requires map
keys to be ordered, we do not use maps to encode maps, to allow streaming those maps to the data stream without
preordering the keys.
