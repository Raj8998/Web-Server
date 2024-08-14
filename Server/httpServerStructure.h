struct HTTP_Request {
  char HTTP_version[100];

  char method[100];
  char url[100];
};

struct HTTP_Response {
  char HTTP_version[100]; // 1.0 for this assignment

  char status_code[100]; // ex: 200, 404, etc.
  char status_text[100]; // ex: OK, Not Found, etc.

  char content_type[100];
  char content_length[100];

  char body[4096];
};