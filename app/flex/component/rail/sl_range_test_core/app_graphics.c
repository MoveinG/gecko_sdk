/***************************************************************************//**
 * @file
 * @brief app_graphics.c
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include <string.h>
#include "dmd.h"
#include "glib.h"
#include "printf.h"
#include "rail.h"
#if defined(SL_CATALOG_RADIO_CONFIG_SIMPLE_RAIL_SINGLEPHY_PRESENT)
#include "sl_rail_util_init.h"
#endif
#include "app_graphics.h"
#include "app_menu.h"
#include "app_measurement.h"
#include "sl_flex_images.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

/// Constants related to the display
#define GRAPHICS_X_SIZE             128u
#define GRAPHICS_FONT_HEIGHT       (glib_context.font.fontHeight)
#define GRAPHICS_MAX_X             (glib_context.pDisplayGeometry->xSize - 1u)

#define GRAPHICS_CHART_HEIGHT       33u
#define GRAPHICS_CHART_X_OFFSET     22u
#define GRAPHICS_CHART_SIG           2u
#define GRAPHICS_CHART_SIG_0_OFFSET  5u
#define GRAPHICS_CHART_SIG_1_OFFSET  17u

#define GRAPHICS_CHART_SIZE         (GRAPHICS_X_SIZE            \
                                     - (GRAPHICS_CHART_X_OFFSET \
                                        + GRAPHICS_CHART_SIG    \
                                        + 1 /* Line width */    \
                                        + 1 /* End border */))

#define GRAPHICS_TRX_CHART_X        -1
#define GRAPHICS_TRX_CHART_Y        15u
#define GRAPHICS_TRX_TEXT_SPACING    1u
#define GRAPHICS_TRX_LINE_SPACING    3u

#define GRAPHICS_TX_START_ROW       16u
#define GRAPHICS_TX_LINE_SPACING     8u
#define GRAPHICS_TX_TEXT_SPACING     4u

#define GRAPHICS_RX_CHART_X         -1
#define GRAPHICS_RX_CHART_Y         15u
#define GRAPHICS_RX_LINE_SPACING     3u
#define GRAPHICS_RX_TEXT_SPACING     1u

/// Indices for the minimum and maximum values (last and first index,
/// respectively).
#define GRAPHICS_RSSI_MIN_INDEX 2u
#define GRAPHICS_RSSI_MAX_INDEX 0u

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
/*******************************************************************************
 * @brief  Function to draw a button icon.
 ******************************************************************************/
static void graphics_draw_button_icon(const uint8_t location, const uint8_t type);

/*******************************************************************************
 * @brief  Function to write a title text in the middle.
 ******************************************************************************/
static void graphics_draw_title_text(char * text, GLIB_Font_t *font);

/*******************************************************************************
 * @brief  Function to draw the menu points.
 ******************************************************************************/
static void graphics_draw_menu_items(uint8_t start_index);

/*******************************************************************************
 * @brief  Function to draw the menu.
 ******************************************************************************/
static void graphics_draw_menu_frame(void);

/*******************************************************************************
 * @brief  Function to write the menu highlight.

 ******************************************************************************/
static void graphics_draw_item_pointer(uint8_t row);

/*******************************************************************************
 * @brief  Function to write the settings are static, in RX and TX modes.

 ******************************************************************************/
static inline uint8_t graphics_draw_constans_info(uint8_t row);

/*******************************************************************************
 * @brief  This function writes the dynamically changing values in RX/TX modes.
 ******************************************************************************/
static uint8_t graphics_draw_volatile_info(uint8_t row);

/*******************************************************************************
 * @brief   Function that returns an element from the given index of the RSSI
 *          history buffer.
 ******************************************************************************/
static uint8_t graphics_get_rssi_value(uint8_t index);

/*******************************************************************************
 * @brief  Function to draw the RSSI history chart in RX mode.
 ******************************************************************************/
static void graphics_draw_rssi_chart(int16_t x, int16_t y);

/*******************************************************************************
 * @brief  Function that draws the RSSI history chart frame.
 ******************************************************************************/
static uint8_t graphics_draw_rssi_chart_frame(int16_t x, int16_t y);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
/// Context used all over the graphics
static GLIB_Context_t glib_context;

/// RSSI graphics options and variables
static const int8_t rssi_chart_axis[3u] = { 10, -40, -100 };
static uint8_t rssi_finger;
static uint8_t rssi_buffer[GRAPHICS_CHART_SIZE];

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
/*******************************************************************************
 * @brief Initializes the graphics stack.
 * @note This function will /hang/ if errors occur (usually
 *       caused by faulty displays.
 ******************************************************************************/
void graphics_init(void)
{
  EMSTATUS status;

  /* Initialize the DMD module for the DISPLAY device driver. */
  status = DMD_init(0);
  if (DMD_OK != status) {
    while (1) ;
  }

  status = GLIB_contextInit(&glib_context);
  if (GLIB_OK != status) {
    while (1) ;
  }

  glib_context.backgroundColor = White;
  glib_context.foregroundColor = Black;

  /* Use Normal font */
  GLIB_setFont(&glib_context, (GLIB_Font_t *)&GLIB_FontNormal8x8);
}

/*******************************************************************************
 * @brief  Function to display the menu.
 * @param  None.
 * @return None.
 ******************************************************************************/
void graphics_draw_menu(void)
{
  GLIB_clear(&glib_context);

  graphics_draw_menu_frame();

  glib_context.foregroundColor = White;
  graphics_draw_title_text("CONFIGURATION", (GLIB_Font_t *) &GLIB_FontNormal8x8);

  GLIB_drawLineH(&glib_context, 0u, (GRAPHICS_FONT_HEIGHT + 2u), GRAPHICS_MAX_X);

  graphics_draw_button_icon(0u, ICON_DOWN);

  graphics_draw_button_icon(1u, menu_item_icon(menu_get_actual_id()));

  graphics_draw_menu_items(menu_get_dispaly_start_id());
  graphics_draw_item_pointer(menu_get_id_with_hidden(menu_get_actual_id()) - menu_get_dispaly_start_id());

  DMD_updateDisplay();
}

/*******************************************************************************
 * @brief  Function that draws the starting screen.
 * @param  None.
 * @return None.
 ******************************************************************************/
void graphics_draw_init_screen(void)
{
  // Heights:
  // Logo: 62 + Title: 8 + Card: 8 + Spacing: 2*5 = 88
  char buff[22u];
  uint32_t base_frequency = 0;
  uint32_t channel_spacing = 0;
  int16_t power = 0;

  get_rail_config_data(&base_frequency, &channel_spacing, &power);

  // Draw init screen
  GLIB_clear(&glib_context);

  GLIB_drawBitmap(&glib_context,
                  0,
                  10,
                  silabsLogo.img_x,
                  silabsLogo.img_y,
                  (uint8_t *) silabsLogo.image_bits);

  GLIB_setFont(&glib_context, (GLIB_Font_t *) &GLIB_FontNormal8x8);
  GLIB_drawStringOnLine(&glib_context, "Range Test", 8, GLIB_ALIGN_CENTER, 0, 0, false);

  GLIB_setFont(&glib_context, (GLIB_Font_t *)&GLIB_FontNarrow6x8);
  if (is_current_phy_standard()) {
    GLIB_drawStringOnLine(&glib_context, "BLE & IEEE802.15.4", 9, GLIB_ALIGN_CENTER, 0, 0, false);
  }

  sprintf(buff,
          "EFR32 %03luMHz %+lidBm",
          (base_frequency / 1000000ul),
          (power / 10));
  GLIB_drawStringOnLine(&glib_context, buff, 10, GLIB_ALIGN_CENTER, 0, 0, false);

  DMD_updateDisplay();
}

/*******************************************************************************
 * @brief  Displays the Range Test in RX mode.
 * @param  None.
 * @return None.
 ******************************************************************************/
void graphics_draw_rx_screen(void)
{
  uint8_t row;

  GLIB_clear(&glib_context);

  graphics_draw_menu_frame();

  glib_context.foregroundColor = White;
  graphics_draw_title_text("Range Test",
                           (GLIB_Font_t *) &GLIB_FontNormal8x8);
  GLIB_drawLineH(&glib_context, 0u, (GRAPHICS_FONT_HEIGHT + 2u), GRAPHICS_MAX_X);

  graphics_draw_button_icon(0u, ICON_BACK);

  glib_context.foregroundColor = Black;
  row = graphics_draw_rssi_chart_frame(GRAPHICS_RX_CHART_X, GRAPHICS_RX_CHART_Y);

  graphics_draw_rssi_chart((GRAPHICS_RX_CHART_X
                            + GRAPHICS_CHART_X_OFFSET
                            + GRAPHICS_CHART_SIG
                            + 1u),
                           GRAPHICS_RX_CHART_Y);

  row += GRAPHICS_RX_LINE_SPACING;
  GLIB_drawLineH(&glib_context, 0u, row, GRAPHICS_MAX_X);
  row += GRAPHICS_RX_LINE_SPACING;

  row += graphics_draw_constans_info(row);

  row += GRAPHICS_RX_TEXT_SPACING;
  GLIB_drawLineH(&glib_context, 0u, row, GRAPHICS_MAX_X);
  row += GRAPHICS_RX_LINE_SPACING;

  graphics_draw_volatile_info(row);

  DMD_updateDisplay();
}

/*******************************************************************************
 * @brief  Displays the Range Test in TX mode.
 * @param  None.
 * @return None.
 ******************************************************************************/
void graphics_draw_tx_screen(void)
{
  uint8_t row;

  GLIB_clear(&glib_context);

  graphics_draw_menu_frame();

  glib_context.foregroundColor = White;
  graphics_draw_title_text("Range Test",
                           (GLIB_Font_t *) &GLIB_FontNormal8x8);
  GLIB_drawLineH(&glib_context, 0u, (GRAPHICS_FONT_HEIGHT + 2u), GRAPHICS_MAX_X);

  graphics_draw_button_icon(1u, ((range_test_measurement.tx_is_running) ? (ICON_STOP) : (ICON_START)));
  graphics_draw_button_icon(0u, ICON_BACK);

  glib_context.foregroundColor = Black;
  row = GRAPHICS_TX_START_ROW;
  row += graphics_draw_constans_info(row);

  row += GRAPHICS_TX_TEXT_SPACING;
  GLIB_drawLineH(&glib_context, 0u, row, GRAPHICS_MAX_X);
  row += GRAPHICS_TX_LINE_SPACING;

  graphics_draw_volatile_info(row);

  DMD_updateDisplay();
}

/*******************************************************************************
 * @brief  Function to clear the RSSI history.
 * @param  None.
 * @return None.
 ******************************************************************************/
void graphics_clear_rssi_buffer()
{
  uint8_t i;

  // Clear RSSI buffer
  for (i = 0u; i < GRAPHICS_CHART_SIZE; i++) {
    // fill up the buffer with minimal values
    rssi_buffer[i] = (uint8_t)(rssi_chart_axis[GRAPHICS_RSSI_MIN_INDEX]);
  }
  // Clear newest pointer
  rssi_finger = 0u;
}

/*******************************************************************************
 * @brief  This function adds a given RSSI value to the RSSI history buffer.
 * @param  rssi: Current RSSI value to add to the RSSI history.
 * @return None.
 ******************************************************************************/
void graphics_add_to_rssi_buffer(uint8_t rssi)
{
  rssi_finger++;
  if (GRAPHICS_CHART_SIZE <= rssi_finger) {
    rssi_finger = 0u;
  }
  rssi_buffer[rssi_finger] = rssi;
}

/*******************************************************************************
 * @brief  This function checks and modifies a given RSSI value to be able to
 * add to the RSSI history buffer.
 * @param  rssi: Current RSSI value to add to the RSSI history.
 * @return display_value: modified value able to be displayed
 ******************************************************************************/
int8_t rssi_value_for_display(int8_t original_value)
{
  int8_t display_value = 0;
  display_value = (rssi_chart_axis[GRAPHICS_RSSI_MIN_INDEX] > original_value)
                  // If lower than minimum -> minimum
                  ? (rssi_chart_axis[GRAPHICS_RSSI_MIN_INDEX])
                  // else check if higher than maximum
                  : ((rssi_chart_axis[GRAPHICS_RSSI_MAX_INDEX] < original_value)
                     // Higher than maximum -> maximum
                     ? (rssi_chart_axis[GRAPHICS_RSSI_MAX_INDEX])
                     // else value is OK
                     : (original_value));
  return display_value;
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------
/*******************************************************************************
 * @brief  Function to draw a button icon.
 *
 * @param  location: Button location.
 *                              0 - Left
 *                              1 - Right.
 * @param  type: Button type to display, values from enum menuItemIcon_e.
 *
 * @return None.
 ******************************************************************************/
static void graphics_draw_button_icon(const uint8_t location, const uint8_t type)
{
  bool bitmap = false;
  char buff[6u];
  uint16_t x, y;

  // Pre-Assume text
  GLIB_setFont(&glib_context, (GLIB_Font_t *) &GLIB_FontNormal8x8);
  x = glib_context.font.fontWidth;
  y = glib_context.font.fontHeight;

  switch (type) {
    case ICON_NONE:
      buff[0u] = '\0';
      x = 0u;         // 0 chars
      break;

    case ICON_SHOW:
      sprintf(buff, "SHOW");
      x = 4u * x;    // SHOW = 4 chars
      break;

    case ICON_START:
      sprintf(buff, "START");
      x = 5u * x;    // START = 5 chars
      break;

    case ICON_STOP:
      sprintf(buff, "STOP");
      x = 4u * x;    // STOP = 4 chars
      break;

    case ICON_BACK:
      sprintf(buff, "BACK");
      x = 4u * x;    // BACK = 4 chars
      break;

    case ICON_GO:
      sprintf(buff, "GO");
      x = 2u * x;    // GO = 2 chars
      break;

    default:
      // Custom icon
      bitmap = true;
      x = icons[type]->img_x;
      y = icons[type]->img_y;
      break;
  }

  if (location) { // RIGHT
    x = (((3 * glib_context.pDisplayGeometry->xSize) / 2u) - x) / 2u;
  } else { // LEFT
    x = ((glib_context.pDisplayGeometry->xSize / 2u) - x) / 2u;
  }
  y = glib_context.pDisplayGeometry->ySize - y - 1u;

  if (bitmap) {
    GLIB_drawBitmap(&glib_context,
                    x,
                    y,
                    icons[type]->img_x,
                    icons[type]->img_y,
                    icons[type]->image_bits);
  } else {
    GLIB_drawString(&glib_context, buff, strlen(buff), x, y, false);
  }
  // Set font back
  GLIB_setFont(&glib_context, (GLIB_Font_t *) &GLIB_FontNarrow6x8);

  // Vertical line that separates them
  GLIB_drawLineV(&glib_context,
                 (glib_context.pDisplayGeometry->xSize / 2u),
                 (glib_context.pDisplayGeometry->ySize - glib_context.font.fontHeight - 2u),
                 glib_context.pDisplayGeometry->ySize);
  // Horizontal line
  GLIB_drawLineH(&glib_context,
                 0u,
                 (glib_context.pDisplayGeometry->ySize - glib_context.font.fontHeight - 2u - 1u),
                 glib_context.pDisplayGeometry->xSize);
}

/*******************************************************************************
 * @brief  Function to write a title text in the middle.
 *
 * @param  text: Pointer to the text to be written.
 * @param  font: Pointer to the font to use.
 *
 * @return None.
 ******************************************************************************/
static void graphics_draw_title_text(char * text, GLIB_Font_t *font)
{
  GLIB_setFont(&glib_context, font);
  GLIB_drawStringOnLine(&glib_context, text, 0, GLIB_ALIGN_CENTER, 0, 2, false);
}

/*******************************************************************************
 * @brief  Function to draw the menu points.
 *
 * @param  start_index: Index of the menu item which is drawn first.
 *
 * @return None.
 ******************************************************************************/
static void graphics_draw_menu_items(uint8_t start_index)
{
  uint8_t i = start_index;
  uint8_t hidden_menus_count = 0;

  glib_context.foregroundColor = Black;
  glib_context.backgroundColor = White;

  // Draw menu items
  GLIB_setFont(&glib_context, (GLIB_Font_t *)&GLIB_FontNarrow6x8);

  while (i < (start_index + GRAPHICS_MENU_DISP_SIZE + hidden_menus_count)) {
    if (!menu_item_is_hidden(i)) {
      GLIB_drawString(&glib_context,
                      menu_item_str(i),
                      strlen(menu_item_str(i)),
                      6u,
                      (16u + (i - start_index - hidden_menus_count) * 8u),
                      false);
    } else {
      hidden_menus_count++;
    }

    if (menu_is_last_item(i)) {
      break;
    }
    i++;
  }
}

/*******************************************************************************
 * @brief  Function to draw the menu.
 * @param  None.
 * @return None.
 ******************************************************************************/
static void graphics_draw_menu_frame(void)
{
  glib_context.foregroundColor = Black;

  GLIB_Rectangle_t glibRect;
  // Title
  glibRect.xMin = glibRect.yMin = 1u;
  glibRect.xMax = glib_context.pDisplayGeometry->xSize - 2u;
  glibRect.yMax = glib_context.font.fontHeight + 2u;
  GLIB_drawRectFilled(&glib_context, &glibRect);
  GLIB_drawLineH(&glib_context, 0u, (glibRect.yMax + 1u), GRAPHICS_MAX_X);
  GLIB_drawLineH(&glib_context, 0u, (glibRect.yMax + 2u), GRAPHICS_MAX_X);

  // Footer
  glibRect.yMax = glib_context.pDisplayGeometry->ySize - 2u;
  glibRect.yMin = glibRect.yMax - glib_context.font.fontHeight - 1u;
  GLIB_drawRectFilled(&glib_context, &glibRect);
  GLIB_drawLineH(&glib_context, 0u, (glibRect.yMin - 1u), GRAPHICS_MAX_X);
  GLIB_drawLineH(&glib_context, 0u, (glibRect.yMin - 2u), GRAPHICS_MAX_X);
}

/*******************************************************************************
 * @brief  Function to write the menu highlight.
 *
 * @param  row: The row on the display the is to be highlighted.
 *
 * @return None.
 ******************************************************************************/
static void graphics_draw_item_pointer(uint8_t row)
{
  GLIB_drawLineH(&glib_context, 1u, ((row * 8u) + 16u), 1u);
  GLIB_drawLineH(&glib_context, 1u, ((row * 8u) + 16u + 1u), 2u);
  GLIB_drawLineH(&glib_context, 1u, ((row * 8u) + 16u + 2u), 3u);
  GLIB_drawLineH(&glib_context, 1u, ((row * 8u) + 16u + 3u), 4u);
  GLIB_drawLineH(&glib_context, 1u, ((row * 8u) + 16u + 4u), 3u);
  GLIB_drawLineH(&glib_context, 1u, ((row * 8u) + 16u + 5u), 2u);
  GLIB_drawLineH(&glib_context, 1u, ((row * 8u) + 16u + 6u), 1u);
}

/*******************************************************************************
 * @brief  Function to write the settings are static, in RX and TX modes.
 *
 * @param  row: First row of the static values to be displayed.
 *
 * @return The next row to write to.
 ******************************************************************************/
static inline uint8_t graphics_draw_constans_info(uint8_t row)
{
  char *ptr;
  char buff[21u];
  uint8_t origRow = row;

  uint32_t base_frequency = 0;
  uint32_t channel_spacing = 0;
  int16_t power = 0;

  get_rail_config_data(&base_frequency, &channel_spacing, &power);

  RAIL_Handle_t rail_handle = get_current_rail_handler();

  if (RADIO_MODE_TX == range_test_settings.radio_mode) {
    // Variables for the requested and actual radio output power
    int16_t power;
    int16_t reqpower;

    // Check if frequency has fractional part in MHz scale
    if ( 0u < (base_frequency % 1000000ul)) {
      sprintf(buff,
              "%u.%02uMHz",
              (uint16_t) (base_frequency / 1000000ul),
              (uint16_t) ((base_frequency % 1000000ul) / 10000u));
    } else {
      sprintf(buff,
              "%uMHz",
              (uint16_t) (base_frequency / 1000000ul));
    }

    ptr = menu_print_aligned("Frequency:", buff, 21u);
    GLIB_drawString(&glib_context, ptr, strlen(ptr), 1u, row, false);
    row += GRAPHICS_FONT_HEIGHT;

    sprintf(buff, "%ukHz", (uint16_t) (channel_spacing / 1000u));
    ptr = menu_print_aligned("Ch. spacing:", buff, 21u);
    GLIB_drawString(&glib_context, ptr, strlen(ptr), 1u, row, false);
    row += GRAPHICS_FONT_HEIGHT;

    sprintf(buff, "%u", range_test_settings.channel);
    ptr = menu_print_aligned("Ch. number:", buff, 21u);
    GLIB_drawString(&glib_context, ptr, strlen(ptr), 1u, row, false);
    row += GRAPHICS_FONT_HEIGHT;

    power = (int16_t) RAIL_GetTxPowerDbm(rail_handle);
    reqpower = range_test_settings.tx_power;
    // Since power is coming in 0.1 dBm units, integer and fraction digits are split
    sprintf(buff,
            "%+i.%d/%+i.%ddBm",
            (reqpower / 10),
            (((reqpower > 0) ? (reqpower) : (-reqpower)) % 10),
            (power / 10),
            (((power > 0) ? (power) : (-power)) % 10));
    ptr = menu_print_aligned("Power:", buff, 21u);
    GLIB_drawString(&glib_context, ptr, strlen(ptr), 1u, row, false);
    row += GRAPHICS_FONT_HEIGHT;

    sprintf(buff, "%u", range_test_settings.source_id);
    ptr = menu_print_aligned("Source ID:", buff, 21u);
    GLIB_drawString(&glib_context, ptr, strlen(ptr), 1u, row, false);
    row += GRAPHICS_FONT_HEIGHT;

    sprintf(buff, "%u", range_test_settings.destination_id);
    ptr = menu_print_aligned("Remote ID:", buff, 21u);
    GLIB_drawString(&glib_context, ptr, strlen(ptr), 1u, row, false);
    row += GRAPHICS_FONT_HEIGHT;
  } else {
    // Buffering volatile fields
    uint8_t  destID = range_test_settings.destination_id;
    uint8_t  srcID = range_test_settings.source_id;

    if (RADIO_MODE_RX == range_test_settings.radio_mode) {
      // Check if frequency has fractional part in MHz scale
      if (0u < base_frequency % 1000000ul) {
        sprintf(buff,
                "F:%uM%02u",
                (uint16_t) (base_frequency / 1000000ul),
                (uint16_t) ((base_frequency % 1000000ul) / 10000u));
      } else {
        sprintf(buff,
                "F:%uMHz",
                (uint16_t) (base_frequency / 1000000ul));
      }

      GLIB_drawString(&glib_context, buff, strlen(buff), 1u, row, false);
      row += GRAPHICS_FONT_HEIGHT;
    }
    sprintf(buff,
            "Fch:%4ukHz  Ch#: %3u",
            (uint16_t) (channel_spacing / 1000u),
            range_test_settings.channel);
    GLIB_drawString(&glib_context, buff, strlen(buff), 1u, row, false);
    row += GRAPHICS_FONT_HEIGHT;

    sprintf(buff, "src: %3u     dst: %3u", srcID, destID);
    GLIB_drawString(&glib_context, buff, strlen(buff), 1u, row, false);
    row += GRAPHICS_FONT_HEIGHT;
  }

  return (row - origRow);
}

/*******************************************************************************
 * @brief  This function writes the dynamically changing values in RX and
 *         TX modes.
 *
 * @param  row: Starting row for displaying the values.
 *
 * @return The next row available to write to.
 ******************************************************************************/
static uint8_t graphics_draw_volatile_info(uint8_t row)
{
  char *ptr;
  char buff[22u];
  uint8_t origRow = row;

  // volatile fields used on multiple places
  uint8_t  radioMode = range_test_settings.radio_mode;
  uint16_t pktsSent = range_test_measurement.packets_sent;

  if (RADIO_MODE_TX == radioMode) {
    sprintf(buff, "%5u", pktsSent);
    ptr = menu_print_aligned("Transmitted:", buff, 21u);
    GLIB_drawString(&glib_context, ptr, strlen(ptr), 1u, row, false);
    row += GRAPHICS_FONT_HEIGHT;
  } else { // RADIO_MODE_TX
           //buffering volatile data fields used only here
    uint16_t pktsCnt = range_test_measurement.packets_received_counter;
    uint16_t pktsRcvd = range_test_measurement.packets_received_correctly;
    uint8_t  rssiLatch = range_test_measurement.rssi_latch_value;
    float    PER = range_test_measurement.PER;
    float    MA = range_test_measurement.moving_average;

    if (RANGETEST_PACKET_COUNT_INVALID == pktsRcvd) {
      sprintf(buff, "---.- dBm");
    } else {
      sprintf(buff, "% 3d dBm", (int8_t)rssiLatch);
    }
    ptr = menu_print_aligned("RSSI:", buff, 21u);
    GLIB_drawString(&glib_context, ptr, strlen(ptr), 1u, row, false);
    row += GRAPHICS_FONT_HEIGHT;

    if (RANGETEST_PACKET_COUNT_INVALID == pktsRcvd) {
      // No packet received yet.
      sprintf(buff, "  -- / -- ");
    } else {
      sprintf(buff, "%5u/%5u", pktsRcvd, pktsCnt);
    }

    ptr = menu_print_aligned("Rx:", buff, 21u);
    GLIB_drawString(&glib_context, ptr, strlen(ptr), 1u, row, false);
    row += GRAPHICS_FONT_HEIGHT;

    if (RANGETEST_PACKET_COUNT_INVALID == pktsRcvd) {
      sprintf(buff, "MA: --.-%%  PER: --.-%%");
    } else {
      sprintf(buff, "MA:%5.1f%%  PER:%5.1f%%", MA, PER);
    }
    GLIB_drawString(&glib_context, buff, strlen(buff), 1u, row, false);
    row += GRAPHICS_FONT_HEIGHT;
  }

  return (row - origRow);
}

/*******************************************************************************
 * @brief   Function that returns an element from the given index of the RSSI
 *          history buffer.
 *
 * @param   index: The index of the element we want to get.
 *
 * @returns Actual value of the element.
 ******************************************************************************/
static uint8_t graphics_get_rssi_value(uint8_t index)
{
  if (GRAPHICS_CHART_SIZE <= index) {
    return 0u;
  }

  if (index > rssi_finger) {
    index -= rssi_finger;
    return (rssi_buffer[GRAPHICS_CHART_SIZE - index]);
  }

  return (rssi_buffer[rssi_finger - index]);
}

/*******************************************************************************
 * @brief  Function to draw the RSSI history chart in RX mode.
 *
 * @param  x: Leftmost pixel column to start drawing from.
 * @param  y: Top pixel row of the chart. The zero RSSI value is is 32 pixels
 *            lower.
 *
 * @return None.
 ******************************************************************************/
static void graphics_draw_rssi_chart(int16_t x, int16_t y)
{
  uint8_t i;

  for (i = 1u; i < GRAPHICS_CHART_SIZE; i++) {
    GLIB_drawLineV(&glib_context,
                   (x + i - 1u),
                   ((y + 32u)
                    - ((int8_t)(graphics_get_rssi_value(GRAPHICS_CHART_SIZE - i))
                       - rssi_chart_axis[GRAPHICS_RSSI_MIN_INDEX])  / 4),
                   (y + 32u));
  }
}

/*******************************************************************************
 * @brief  Function that draws the RSSI history chart frame.
 *
 * @param  x: Base location X coordinate. Chart is drawn to the left
 *               relative to this position.
 * @param  y: Base location Y coordinate.
 *
 * @return Next free row.
 ******************************************************************************/
static uint8_t graphics_draw_rssi_chart_frame(int16_t x, int16_t y)
{
  char buff[5u];

  // Draw RSSI chart frame
  GLIB_drawLineV(&glib_context,
                 (x + GRAPHICS_CHART_X_OFFSET + GRAPHICS_CHART_SIG),
                 y,
                 (y + GRAPHICS_CHART_HEIGHT));
  GLIB_drawLineH(&glib_context,
                 (x + GRAPHICS_CHART_X_OFFSET),
                 (y + GRAPHICS_CHART_SIG_0_OFFSET),
                 (x + GRAPHICS_CHART_X_OFFSET + GRAPHICS_CHART_SIG));
  GLIB_drawLineH(&glib_context,
                 (x + GRAPHICS_CHART_X_OFFSET),
                 (y + GRAPHICS_CHART_SIG_1_OFFSET),
                 (x + GRAPHICS_CHART_X_OFFSET + GRAPHICS_CHART_SIG));
  GLIB_drawLineH(&glib_context,
                 (x + GRAPHICS_CHART_X_OFFSET),
                 (y + GRAPHICS_CHART_HEIGHT),
                 GRAPHICS_MAX_X);

  sprintf(buff, "% 3d", rssi_chart_axis[0u]);
  GLIB_drawString(&glib_context, buff, strlen(buff), x, (y + GRAPHICS_CHART_SIG_0_OFFSET - 3u), false);
  sprintf(buff, "% 3d", rssi_chart_axis[1u]);
  GLIB_drawString(&glib_context, buff, strlen(buff), x, (y + GRAPHICS_CHART_SIG_1_OFFSET - 3u), false);
  sprintf(buff, "% 3d", rssi_chart_axis[2u]);
  GLIB_drawString(&glib_context, buff, strlen(buff), x, (y + 3u * GRAPHICS_FONT_HEIGHT), false);

  return (y + GRAPHICS_CHART_HEIGHT); // End row
}
