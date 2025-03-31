#include <gtest/gtest.h>

#include "server.h"

Client *client = new Client(0);
Server *server = new Server(0);

TEST(login, userExists){
    base["user"] = "password";
    EXPECT_EQ(client->login("user", "password"), USER_EXISTS);
}

TEST(login, wrongPassword){
    base["user"] = "password";
    EXPECT_EQ(client->login("user", "wrongPass"), WRONG_PASSWORD);
}

TEST(login, userNotExists){
    EXPECT_EQ(client->login("admin", "admin"), USER_NOT_EXISTS);
}

TEST(registeration, newUser){
    EXPECT_EQ(client->reg("admin", "root"), USER_REGISTERED);
    EXPECT_EQ(client->login("admin", "root"), USER_EXISTS);
}

TEST(registeration, newUserAlreadyExists){
    EXPECT_EQ(client->reg("admin", "root"), USER_ALREADY_EXISTS);
}

TEST(base, openAndRead){
    base.clear();
    EXPECT_EQ(server->readBase(), true);
}

TEST(base, openAndWrite){
    EXPECT_EQ(server->writeBase(), true);
}

TEST(threads, multithreading){
    vector<IClient*> thr;
    for(int i = 0; i < 5; i++){
        thr.push_back(new Client(i));
        EXPECT_TRUE(thr[i]);
    }
}

int main(){
    testing::InitGoogleTest();
    //writeBase();
    return RUN_ALL_TESTS();
}
