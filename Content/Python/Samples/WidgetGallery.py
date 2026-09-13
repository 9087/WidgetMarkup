from WidgetMarkupComponent import WidgetMarkupComponent, reactive


class WidgetGallery(WidgetMarkupComponent):
    """Gallery of every styled widget, used to compare WidgetMarkup with Slate.

    All visuals come from /WidgetMarkup/Core/Styles/StarshipStyle (editor layer),
    which inherits /WidgetMarkup/Core/Styles/StarshipCoreStyle.
    """

    @reactive
    def combo_options(self):
        """Option list bound from Python instead of written in the XML."""
        return ["ComboBox", "Second entry"]

    @reactive
    def combo_selection(self):
        """Selected entry bound from Python; it is one of combo_options."""
        return "Second entry"
