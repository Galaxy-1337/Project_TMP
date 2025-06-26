#ifndef PROTOCOL_DEFINES_H
#define PROTOCOL_DEFINES_H

namespace JsonFields {
constexpr char TYPE[] = "type";
constexpr char LOGIN[] = "login";
constexpr char USERNAME[] = "username";
constexpr char PASSWORD[] = "password";
constexpr char PASSWORD_HASH[] = "password_hash";
constexpr char STATUS[] = "status";
constexpr char MESSAGE[] = "message";
constexpr char ROOM_ID[] = "room_id";
constexpr char PLAYERS[] = "players";
constexpr char OWNER[] = "owner";
constexpr char LOBBY_LIST[] = "lobbies";
constexpr char AUTH_TOKEN[] = "auth_token"; // Добавлено

// Игровые поля
constexpr char ROLE_SCOUT[] = "scout";
constexpr char ROLE_SABOTEUR[] = "saboteur";
constexpr char ROW[] = "row";
constexpr char COL[] = "col";
constexpr char ROLE[] = "role";
constexpr char FIELD[] = "field";
constexpr char CURRENT_TURN[] = "currentTurn";
constexpr char SCOUT_SCORE[] = "scoutScore";
constexpr char SABOTEUR_SCORE[] = "saboteurScore";
constexpr char WINNER[] = "winner";
constexpr char ROWS[] = "rows";
constexpr char COLS[] = "cols";
constexpr char MINES_AROUND[] = "minesAround";
constexpr char HAS_MINE[] = "hasMine";
constexpr char STATE[] = "state";
}

namespace MessageType {
// Системные сообщения
constexpr int AUTH_REQUEST = 0;
constexpr int AUTH_RESPONSE = 1;
constexpr int REGISTER_REQUEST = 2;
constexpr int CREATE_LOBBY = 3;
constexpr int LOBBY_CREATED = 4;
constexpr int JOIN_LOBBY = 5;
constexpr int LOBBY_UPDATE = 6;
constexpr int GAME_START = 7;
constexpr int GET_LOBBIES = 8;
constexpr int LOBBY_LIST = 9;
constexpr int JOIN_RESULT = 10;
constexpr int LOBBY_CLOSED = 11;

// Игровые сообщения
constexpr int GAME_UPDATE = 12;
constexpr int GAME_OVER = 13;
constexpr int GAME_MOVE = 14;
constexpr int SURRENDER = 15;
constexpr int ROLE_ASSIGNED = 16;
}

#endif // PROTOCOL_DEFINES_H
