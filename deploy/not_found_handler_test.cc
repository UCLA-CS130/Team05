#include "gtest/gtest.h"
#include "not_found_handler.h"
#include "request.h"
#include "response.h"

TEST(NotFoundHandlerTest, NotFound) {

    Request request;
    Response actual_response;

    NotFoundHandler h;
    EXPECT_EQ(RequestHandler::Status::OK, h.HandleRequest(request, &actual_response));

    Response expected_response = Response::DefaultResponse(Response::not_found);


    EXPECT_EQ(expected_response.GetHeaders().size(), actual_response.GetHeaders().size());
    EXPECT_EQ(expected_response.GetStatus(), actual_response.GetStatus());
    EXPECT_EQ(expected_response.GetBody(), actual_response.GetBody());
  
}