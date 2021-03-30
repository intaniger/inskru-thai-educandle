#define DRAW_CMD_REL_LINE_TO 0
#define DRAW_CMD_END_OF_CHUNK 4
#define DRAW_CMD_ABS_MOVE_TO 5
#define DRAW_CMD_ABS_LINE_TO 6
#define DRAW_CMD_BACK_TO_FIRST_POINT 14
#define DRAW_CMD_REL_H_LINE_TO 30
#define DRAW_CMD_REL_V_LINE_TO 62
#define DRAW_CMD_EOF 63

//   switch (i)
//   {
//   case DRAW_CMD_REL_LINE_TO:
//     cmd = 'l';
//     break;
//   case DRAW_CMD_END_OF_CHUNK:
//     cmd = 't';
//     break;
//   case DRAW_CMD_ABS_MOVE_TO:
//     cmd = 'M';
//     break;

//   case DRAW_CMD_ABS_LINE_TO:
//     cmd = 'L';
//     break;

//   case DRAW_CMD_BACK_TO_FIRST_POINT:
//     cmd = 'z';
//     break;

//   case DRAW_CMD_REL_H_LINE_TO:
//     cmd = 'h';
//     break;

//   case DRAW_CMD_REL_V_LINE_TO:
//     cmd = 'v';
//     break;

//   case DRAW_CMD_EOF:
//     cmd = 'T';
//     break;

//   default:
//     break;
//   }
//   printf("%c\n", cmd);
// }