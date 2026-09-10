from WidgetMarkupComponent import WidgetMarkupComponent, reactive


class TestChild(WidgetMarkupComponent):
    @reactive
    def display_text(self):
        return ""

    def on_data_refresh(self, data):
        self.display_text = str(data)

    def on_clicked(self) -> None:
        pass
