#include "gui/Label.h"
#include "gui/tiled_panel.h"

std::pair<std::unique_ptr<Panel>, Label*> create_tooltip(Anchor::Type tooltip_anchor = Anchor::BOTTOM | Anchor::LEFT);
std::pair<std::unique_ptr<TiledPanel>, Label*> create_tooltip_paper(Anchor::Type tooltip_anchor = Anchor::BOTTOM | Anchor::LEFT, Widget* tile_size_reference = nullptr);
