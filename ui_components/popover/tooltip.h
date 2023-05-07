#pragma once

#include "popover.h"

namespace nim_comp {

class TooltipBody :public PopoverBody
{
public:
  explicit TooltipBody(const std::wstring& content);
  ~TooltipBody() = default;

  std::wstring GetType() const override;
  virtual ui::UiSize EstimateSize(ui::UiSize szAvailable) override;
  void DoInit() override;
};

}