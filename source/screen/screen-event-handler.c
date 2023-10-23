#include "../engine.h"
#include "../screen.h"

extern Square screen_pixels_square(SDL_Rect boardRect, int width, int height);

extern Move create_move(U64 boards[12], Square sourceSquare, Square targetSquare);

extern SDL_Rect board_rect(int screenWidth, int screenHeight);
 
extern void screen_board_textures_destroy(ScreenBoardTextures* boardTextures);

extern void screen_board_textures_create(ScreenBoardTextures* boardTextures, Screen screen, Position position);

// When the mouse is clicked:
// - all marks disappear
void screen_event_mouse_down_left_handler(Screen* screen, Position* position, SDL_Event event)
{
  screen->board.meta.markedSquaresBoard = 0ULL;

  texture_destroy(&screen->board.textures.marks);


  Square square = screen_pixels_square(screen->board.rect, event.button.x, event.button.y);


  // If a square is marked and the user clicks on another square, try to make that move
  if(screen->board.meta.markedSquare != SQUARE_NONE && screen->board.meta.markedSquare != square)
  {
    // Try to make the move (markedSquare -> square);
    Move move = create_move(position->boards, screen->board.meta.markedSquare, square);

    if(move_fully_legal(*position, move))
    {
      make_move(position, move);

      screen->board.meta.markedSquare = SQUARE_NONE;
      screen->board.meta.grabbedSquare = SQUARE_NONE;


      texture_destroy(&screen->board.textures.moves);

      texture_destroy(&screen->board.textures.check);

      check_texture_create(&screen->board.textures.check, screen->renderer, screen->board.rect.w, screen->board.rect.h, *position);

      texture_destroy(&screen->board.textures.pieces);

      pieces_texture_create(&screen->board.textures.pieces, screen->renderer, screen->board.rect.w, screen->board.rect.h, *position, SQUARE_NONE);
    }
  }

  // If the user cant make a move or it was an illegal move

  Piece piece = boards_square_piece(position->boards, square); 

  if(piece != PIECE_NONE) 
  {
    screen->board.meta.grabbedSquare = square;
    screen->board.meta.holdingPiece = piece;

    texture_destroy(&screen->board.textures.pieces);

    pieces_texture_create(&screen->board.textures.pieces, screen->renderer, screen->board.rect.w, screen->board.rect.h, *position, screen->board.meta.grabbedSquare);

    texture_destroy(&screen->board.textures.moves);

    moves_texture_create(&screen->board.textures.moves, screen->renderer, screen->board.rect.w, screen->board.rect.h, *position, screen->board.meta.grabbedSquare);
  }
  else
{
    screen->board.meta.markedSquare = SQUARE_NONE;
    screen->board.meta.grabbedSquare = SQUARE_NONE;
  }
}

void screen_event_mouse_down_right_handler(Screen* screen, Position* position, SDL_Event event)
{
  Square square = screen_pixels_square(screen->board.rect, event.button.x, event.button.y);

  screen->board.meta.rightHoldingSquare = square;
}

void screen_event_mouse_down_handler(Screen* screen, Position* position, SDL_Event event)
{
  switch(event.button.button)
  {
    case SDL_BUTTON_LEFT:
      screen_event_mouse_down_left_handler(screen, position, event);
      break;

    case SDL_BUTTON_RIGHT:
      screen_event_mouse_down_right_handler(screen, position, event);
      break;
  }
}

void screen_event_mouse_up_left_handler(Screen* screen, Position* position, SDL_Event event)
{
  Square square = screen_pixels_square(screen->board.rect, event.button.x, event.button.y);

  if(screen->board.meta.grabbedSquare != SQUARE_NONE && square != screen->board.meta.grabbedSquare)
  {
    // Try to make the move (grabbedSquare -> square);
    Move move = create_move(position->boards, screen->board.meta.grabbedSquare, square);

    if(move_fully_legal(*position, move))
    {
      make_move(position, move);

      screen->board.meta.markedSquare = SQUARE_NONE;
      screen->board.meta.grabbedSquare = SQUARE_NONE;


      texture_destroy(&screen->board.textures.moves);

      texture_destroy(&screen->board.textures.check);

      check_texture_create(&screen->board.textures.check, screen->renderer, screen->board.rect.w, screen->board.rect.h, *position);
    }
  }

  // If you are already holding a piece and release it on the same square,
  // stop holding it
  if(screen->board.meta.markedSquare != SQUARE_NONE && screen->board.meta.markedSquare == square)
  {
    screen->board.meta.markedSquare = SQUARE_NONE;
    screen->board.meta.grabbedSquare = SQUARE_NONE;

    texture_destroy(&screen->board.textures.moves);
  }
  else if(square != screen->board.meta.markedSquare && screen->board.meta.grabbedSquare != SQUARE_NONE)
  {
    screen->board.meta.markedSquare = screen->board.meta.grabbedSquare;
  }

  if(screen->board.meta.holdingPiece != PIECE_NONE)
  {
    screen->board.meta.holdingPiece = PIECE_NONE;

    texture_destroy(&screen->board.textures.pieces);

    pieces_texture_create(&screen->board.textures.pieces, screen->renderer, screen->board.rect.w, screen->board.rect.h, *position, SQUARE_NONE);
  }
}

void screen_event_mouse_up_right_handler(Screen* screen, Position* position, SDL_Event event)
{
  Square square = screen_pixels_square(screen->board.rect, event.button.x, event.button.y);

  if(screen->board.meta.rightHoldingSquare != SQUARE_NONE && square != SQUARE_NONE)
  {
    if(screen->board.meta.rightHoldingSquare == square)
    {
      if(BOARD_SQUARE_GET(screen->board.meta.markedSquaresBoard, square))
      {
        screen->board.meta.markedSquaresBoard = BOARD_SQUARE_POP(screen->board.meta.markedSquaresBoard, square);
      }
      else screen->board.meta.markedSquaresBoard = BOARD_SQUARE_SET(screen->board.meta.markedSquaresBoard, square);

      texture_destroy(&screen->board.textures.marks);

      marks_texture_create(&screen->board.textures.marks, screen->renderer, screen->board.rect.w, screen->board.rect.h, screen->board.meta.markedSquaresBoard);
    }
    else
  {
      fprintf(stdout, "Create arrow: (%d -> %d)\n", screen->board.meta.rightHoldingSquare, square);
    }
  }

  screen->board.meta.rightHoldingSquare = SQUARE_NONE;
}

void screen_event_mouse_up_handler(Screen* screen, Position* position, SDL_Event event)
{
  switch(event.button.button)
  {
    case SDL_BUTTON_LEFT:
      screen_event_mouse_up_left_handler(screen, position, event);
      break;

    case SDL_BUTTON_RIGHT:
      screen_event_mouse_up_right_handler(screen, position, event);
      break;
  }
}

void screen_event_key_down_handler(Screen* screen, Position* position, SDL_Event event)
{
  switch(event.key.keysym.sym)
  {
    case SDLK_SPACE:
      // Pause or resume music
      // music_toggle();
      break;
  }
}

void screen_event_mouse_motion_handler(Screen* screen, Position* position, SDL_Event event)
{
  // If holding a piece, update the piece's position

  // fprintf(stdout, "Mouse: (%d, %d)\n", event.motion.x, event.motion.y);

  screen->mouseX = event.motion.x;
  screen->mouseY = event.motion.y;
}

// When the window resizes:
// - change screen dimensions
// - update screen rects
// - update every texture
void screen_event_window_resize_handler(Screen* screen, Position* position, SDL_Event event)
{
  screen->width = event.window.data1;
  screen->height = event.window.data2;

  // SDL_SetWindowSize(screen->window, newWidth, newHeight);

  screen->board.rect = board_rect(screen->width, screen->height);

  screen_board_textures_destroy(&screen->board.textures);

  screen_board_textures_create(&screen->board.textures, *screen, *position);
}

void screen_event_window_handler(Screen* screen, Position* position, SDL_Event event)
{
  if(event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
  {
    screen_event_window_resize_handler(screen, position, event);
  }
}

void screen_event_handler(Screen* screen, Position* position, SDL_Event event)
{
  switch(event.type)
  {
    case SDL_MOUSEBUTTONDOWN:
      screen_event_mouse_down_handler(screen, position, event);
      break;

    case SDL_MOUSEBUTTONUP:
      screen_event_mouse_up_handler(screen, position, event);
      break;

    case SDL_KEYDOWN:
      screen_event_key_down_handler(screen, position, event);
      break;

    case SDL_MOUSEMOTION:
      screen_event_mouse_motion_handler(screen, position, event);
      break;

    case SDL_WINDOWEVENT:
      screen_event_window_handler(screen, position, event);
      break;
  }
}