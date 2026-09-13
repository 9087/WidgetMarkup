import widget_markup
from WidgetMarkupComponent import reactive
from Tests.TestComponent import TestComponent


class TestRichTextBlock(TestComponent):
    """RichTextBlock basics; the DecoratorClasses setter has no input path yet.

    URichTextBlock builds its decorators (and a style set) from DecoratorClasses, so a
    write only takes effect through SetDecorators() - which is what the setter calls.
    Nothing can produce a UClass value for that array today, so this covers the widget
    itself and leaves the decorator path to the test that can write one.
    """

    @reactive
    def rich_text(self):
        return "Rich text"

    def __init__(self):
        try:
            super().__init__("TestRichTextBlock")
            uw = getattr(self, "_widget_markup_user_widget", None)
            self.check_not_none(uw, "user widget loaded")

            text_block = self.find_widget("RichText")
            self.check_not_none(text_block, "rich text block found")
            self.check_equal(text_block.get_editor_property("Text"), "Rich text", "text binding")
            self.check_equal(
                len(list(text_block.get_editor_property("DecoratorClasses"))), 0, "no decorators by default"
            )
            self.report()
        finally:
            if widget_markup.Application.is_test_mode():
                widget_markup.Application.request_shutdown()
