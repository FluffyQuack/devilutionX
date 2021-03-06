/**
 * @file automap.h
 *
 * Interface of the in-game map overlay.
 */
#pragma once

#include <stdint.h>

#include "engine.h"
#include "gendung.h"

namespace devilution {

/** Specifies whether the automap is enabled. */
extern bool automapflag;
/** Tracks the explored areas of the map. */
extern bool automapview[DMAXX][DMAXY];
/** Specifies the scale of the automap. */
extern int AutoMapScale;
extern int AutoMapXOfs;
extern int AutoMapYOfs;
extern int AmLine64;
extern int AmLine32;
extern int AmLine16;
extern int AmLine8;
extern int AmLine4;

/**
 * @brief Initializes the automap.
 */
void InitAutomapOnce();

/**
 * @brief Loads the mapping between tile IDs and automap shapes.
 */
void InitAutomap();

/**
 * @brief Displays the automap.
 */
void StartAutomap();

/**
 * @brief Scrolls the automap upwards.
 */
void AutomapUp();

/**
 * @brief Scrolls the automap downwards.
 */
void AutomapDown();

/**
 * @brief Scrolls the automap leftwards.
 */
void AutomapLeft();

/**
 * @brief Scrolls the automap rightwards.
 */
void AutomapRight();

/**
 * @brief Increases the zoom level of the automap.
 */
void AutomapZoomIn();

/**
 * @brief Decreases the zoom level of the automap.
 */
void AutomapZoomOut();

/**
 * @brief Renders the automap to the given buffer.
 */
void DrawAutomap(const CelOutputBuffer &out);

/**
 * @brief Marks the given coordinate as within view on the automap.
 */
void SetAutomapView(int x, int y);

/**
 * @brief Resets the zoom level of the automap.
 */
void AutomapZoomReset();

} // namespace devilution
