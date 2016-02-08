#ifndef TEST_MSG_SCHEMA_STRUCT
#define TEST_MSG_SCHEMA_STRUCT

msg test_submsg_t {
    sval short; /*!< This is comment */
    lval_neco long;
    i16val int16;
    arr int[];
}

/**
 * This is also comment
 */
msg test_msg_t {
    ival int {-1}; /*!< Default value is -1 */
    dval double[];
    sub test_submsg_t;
    subs test_submsg_t[];
    lval double[];
}

msg test_msg2_arr_t {
    ai8 int8[];
    au8 uint8[];
    ai16 int16[];
    au16 uint16[];
    ai32 int32[];
    au32 uint32[];
    ai64 int64[];
    au64 uint64[];
    ac char[];
    auc uchar[];
    as short[];
    aus short[];
    ai int[];
    aui uint[];
    al long[];
    aul ulong[];
    af float[];
    ad double[];
}

msg test_msg2_t {
    i8 int8 {-1};
    u8 uint8;
    i16 int16;
    u16 uint16;
    i32 int32;
    u32 uint32;
    i64 int64;
    u64 uint64;
    c char;
    uc uchar;
    s short;
    us short;
    i int;
    ui uint;
    l long;
    ul ulong {122};
    f float;
    d double;
    sub test_submsg_t;
    subs test_submsg_t[];
    subarr test_msg2_arr_t;
}

#endif /* TEST_MSG_SCHEMA_STRUCT */
