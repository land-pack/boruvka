#ifndef TEST_MSG_SCHEMA_H
#define TEST_MSG_SCHEMA_H

TEST(testMsgSchemaInit);
TEST(testMsgSchema);

TEST_SUITE(TSMsgSchema) {
    TEST_ADD(testMsgSchemaInit),
    TEST_ADD(testMsgSchema),
    TEST_SUITE_CLOSURE
};

#endif /* TEST_MSG_SCHEMA_H */
