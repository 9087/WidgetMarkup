from __future__ import annotations

import ast
import math
import operator
import random
from typing import Callable

import unreal

from WidgetMarkupComponent import WidgetMarkupComponent, reactive, computed

_BINARY_OPERATORS = {
    ast.Add: operator.add,
    ast.Sub: operator.sub,
    ast.Mult: operator.mul,
    ast.Div: operator.truediv,
    ast.FloorDiv: operator.floordiv,
    ast.Mod: operator.mod,
    ast.Pow: operator.pow,
}

_UNARY_OPERATORS = {
    ast.UAdd: operator.pos,
    ast.USub: operator.neg,
}


class ScientificCalculator(WidgetMarkupComponent):
    @reactive
    def display(self):
        return "0"

    @reactive
    def angle_mode_label(self):
        return "Deg"

    @reactive
    def second_active(self):
        return False

    @computed
    def second_button_color(self):
        if self.second_active:
            return unreal.LinearColor(0.55, 0.55, 0.55, 1.0)
        return unreal.LinearColor(0.97, 0.97, 0.95, 1.0)

    @computed
    def second_button_text_color(self):
        if self.second_active:
            return unreal.SlateColor(unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
        return unreal.SlateColor(unreal.LinearColor(0.22, 0.22, 0.22, 1.0))

    def __init__(self) -> None:
        super().__init__()
        self._expression = ""
        self._memory = 0.0
        self._use_degrees = True
        self._fresh_entry = True
        self._pending_root = False
        self._root_index = 0.0

    def _parse_display(self) -> float:
        text = self.display.replace(",", "")
        if not text or text == "Error":
            return 0.0
        return float(text)

    def _format_number(self, value: float) -> str:
        if math.isnan(value) or math.isinf(value):
            return "Error"
        if abs(value) < 1e-12:
            value = 0.0
        text = f"{value:.10g}"
        if len(text) > 12:
            text = f"{value:.6e}"
        return text

    def _set_display(self, value: float) -> None:
        self.display = self._format_number(value)
        self._fresh_entry = True

    def _append_digit(self, digit: str) -> None:
        if self.display == "Error":
            self.display = digit
            self._expression = ""
            self._fresh_entry = False
            return

        if self._fresh_entry:
            self.display = digit
            self._fresh_entry = False
            return

        if self.display == "0" and digit != "0":
            self.display = digit
            return

        if self.display == "0" and digit == "0":
            return

        if len(self.display.replace(".", "").replace("e", "").replace("-", "")) >= 12:
            return

        self.display = self.display + digit

    def _append_decimal(self) -> None:
        if self.display == "Error":
            self.display = "0."
            self._expression = ""
            self._fresh_entry = False
            return

        if self._fresh_entry:
            self.display = "0."
            self._fresh_entry = False
            return

        if "." in self.display.split("e")[0].split("E")[0]:
            return

        self.display = self.display + "."

    def _commit_display_to_expression(self) -> None:
        if self._expression and self._expression[-1] not in "+-*/%^(":
            return
        self._expression += self.display

    def _append_operator(self, operator_symbol: str) -> None:
        if self.display == "Error":
            return

        if self._expression and not self._fresh_entry:
            self._commit_display_to_expression()
        elif not self._expression:
            self._expression = self.display

        self._expression += operator_symbol
        self._fresh_entry = True

    def _evaluate_expression(self, expression: str) -> float:
        expression = expression.replace("×", "*").replace("÷", "/").replace("^", "**")
        tree = ast.parse(expression, mode="eval")
        return float(self._evaluate_node(tree.body))

    def _evaluate_node(self, node: ast.AST) -> float:
        if isinstance(node, ast.Constant):
            if isinstance(node.value, (int, float)):
                return float(node.value)
            raise ValueError("Unsupported constant")

        if isinstance(node, ast.BinOp):
            left = self._evaluate_node(node.left)
            right = self._evaluate_node(node.right)
            operator_type = type(node.op)
            if operator_type not in _BINARY_OPERATORS:
                raise ValueError("Unsupported operator")
            return _BINARY_OPERATORS[operator_type](left, right)

        if isinstance(node, ast.UnaryOp):
            operand = self._evaluate_node(node.operand)
            operator_type = type(node.op)
            if operator_type not in _UNARY_OPERATORS:
                raise ValueError("Unsupported unary operator")
            return _UNARY_OPERATORS[operator_type](operand)

        if isinstance(node, ast.Call):
            if not isinstance(node.func, ast.Name):
                raise ValueError("Unsupported function call")
            function_name = node.func.id
            function = self._function_map().get(function_name)
            if function is None:
                raise ValueError(f"Unknown function: {function_name}")
            arguments = [self._evaluate_node(argument) for argument in node.args]
            return float(function(*arguments))

        if isinstance(node, ast.Name):
            if node.id == "pi":
                return math.pi
            raise ValueError(f"Unknown name: {node.id}")

        raise ValueError("Unsupported expression")

    def _to_radians(self, value: float) -> float:
        if self._use_degrees:
            return math.radians(value)
        return value

    def _from_radians(self, value: float) -> float:
        if self._use_degrees:
            return math.degrees(value)
        return value

    def _factorial(self, value: float) -> float:
        if value < 0 or abs(value - round(value)) > 1e-9:
            raise ValueError("Factorial requires a non-negative integer")
        return float(math.factorial(int(round(value))))

    def _function_map(self) -> dict[str, Callable[..., float]]:
        return {
            "sin": lambda value: math.sin(self._to_radians(value)),
            "cos": lambda value: math.cos(self._to_radians(value)),
            "tan": lambda value: math.tan(self._to_radians(value)),
            "asin": lambda value: self._from_radians(math.asin(value)),
            "acos": lambda value: self._from_radians(math.acos(value)),
            "atan": lambda value: self._from_radians(math.atan(value)),
            "sinh": math.sinh,
            "cosh": math.cosh,
            "tanh": math.tanh,
            "asinh": math.asinh,
            "acosh": math.acosh,
            "atanh": math.atanh,
            "log": math.log10,
            "ln": math.log,
            "exp": math.exp,
            "sqrt": math.sqrt,
            "fact": self._factorial,
            "root": lambda y, x: x ** (1.0 / y),
        }

    def _resolve_function_name(self, primary: str, secondary: str) -> str:
        if self.second_active:
            self.second_active = False
            return secondary
        return primary

    def _apply_unary_function(self, function_name: str) -> None:
        if self.display == "Error":
            return
        try:
            value = self._parse_display()
            function = self._function_map()[function_name]
            result = function(value)
            self._set_display(result)
            self._expression = self.display
        except Exception:
            self.display = "Error"
            self._expression = ""
            self._fresh_entry = True

    def press_second(self) -> None:
        self.second_active = not self.second_active

    def press_left_paren(self) -> None:
        if self.display == "Error":
            self.display = "0"
            self._expression = ""
        if not self._fresh_entry:
            self._commit_display_to_expression()
        self._expression += "("
        self._fresh_entry = True

    def press_right_paren(self) -> None:
        if self.display == "Error":
            return
        if not self._fresh_entry:
            self._commit_display_to_expression()
        self._expression += ")"
        self._fresh_entry = True

    def press_percent(self) -> None:
        if self.display == "Error":
            return
        try:
            value = self._parse_display() / 100.0
            self._set_display(value)
        except Exception:
            self.display = "Error"

    def press_memory_clear(self) -> None:
        self._memory = 0.0

    def press_memory_add(self) -> None:
        if self.display == "Error":
            return
        self._memory += self._parse_display()

    def press_memory_subtract(self) -> None:
        if self.display == "Error":
            return
        self._memory -= self._parse_display()

    def press_memory_recall(self) -> None:
        self._set_display(self._memory)

    def press_inverse(self) -> None:
        if self.display == "Error":
            return
        try:
            value = self._parse_display()
            if abs(value) < 1e-12:
                raise ZeroDivisionError
            self._set_display(1.0 / value)
            self._expression = self.display
        except Exception:
            self.display = "Error"
            self._expression = ""

    def press_square(self) -> None:
        if self.display == "Error":
            return
        value = self._parse_display()
        self._set_display(value * value)
        self._expression = self.display

    def press_cube(self) -> None:
        if self.display == "Error":
            return
        value = self._parse_display()
        self._set_display(value * value * value)
        self._expression = self.display

    def press_power(self) -> None:
        self._append_operator("**")

    def press_clear(self) -> None:
        self._expression = ""
        self.display = "0"
        self._fresh_entry = True

    def press_negate(self) -> None:
        if self.display == "Error":
            return
        if self.display.startswith("-"):
            self.display = self.display[1:]
        else:
            self.display = "-" + self.display
        self._fresh_entry = False

    def press_divide(self) -> None:
        self._append_operator("/")

    def press_multiply(self) -> None:
        self._append_operator("*")

    def press_subtract(self) -> None:
        self._append_operator("-")

    def press_add(self) -> None:
        self._append_operator("+")

    def press_factorial(self) -> None:
        function_name = self._resolve_function_name("fact", "fact")
        self._apply_unary_function(function_name)

    def press_sqrt(self) -> None:
        function_name = self._resolve_function_name("sqrt", "square")
        if function_name == "square":
            self.press_square()
            return
        self._apply_unary_function("sqrt")

    def press_nth_root(self) -> None:
        if self.second_active:
            self.second_active = False
            self.press_cube()
            return
        if self.display == "Error":
            return
        self._root_index = self._parse_display()
        self._pending_root = True
        self.display = "0"
        self._fresh_entry = True

    def press_log10(self) -> None:
        function_name = self._resolve_function_name("log", "exp")
        if function_name == "exp":
            self._apply_unary_function("exp")
            return
        self._apply_unary_function("log")

    def press_sin(self) -> None:
        function_name = self._resolve_function_name("sin", "asin")
        self._apply_unary_function(function_name)

    def press_cos(self) -> None:
        function_name = self._resolve_function_name("cos", "acos")
        self._apply_unary_function(function_name)

    def press_tan(self) -> None:
        function_name = self._resolve_function_name("tan", "atan")
        self._apply_unary_function(function_name)

    def press_ln(self) -> None:
        function_name = self._resolve_function_name("ln", "exp")
        if function_name == "exp":
            self._apply_unary_function("exp")
            return
        self._apply_unary_function("ln")

    def press_sinh(self) -> None:
        function_name = self._resolve_function_name("sinh", "asinh")
        self._apply_unary_function(function_name)

    def press_cosh(self) -> None:
        function_name = self._resolve_function_name("cosh", "acosh")
        self._apply_unary_function(function_name)

    def press_tanh(self) -> None:
        function_name = self._resolve_function_name("tanh", "atanh")
        self._apply_unary_function(function_name)

    def press_exp(self) -> None:
        self._apply_unary_function("exp")

    def press_degree(self) -> None:
        self._use_degrees = not self._use_degrees
        self.angle_mode_label = "Deg" if self._use_degrees else "Rad"

    def press_pi(self) -> None:
        self._set_display(math.pi)
        self._expression = self.display

    def press_ee(self) -> None:
        if self.display == "Error":
            self.display = "0"
            self._expression = ""
        if "e" in self.display.lower():
            return
        self.display = self.display + "e0"
        self._fresh_entry = False

    def press_random(self) -> None:
        self._set_display(random.random())
        self._expression = self.display

    def press_digit_0(self) -> None:
        self._append_digit("0")

    def press_digit_1(self) -> None:
        self._append_digit("1")

    def press_digit_2(self) -> None:
        self._append_digit("2")

    def press_digit_3(self) -> None:
        self._append_digit("3")

    def press_digit_4(self) -> None:
        self._append_digit("4")

    def press_digit_5(self) -> None:
        self._append_digit("5")

    def press_digit_6(self) -> None:
        self._append_digit("6")

    def press_digit_7(self) -> None:
        self._append_digit("7")

    def press_digit_8(self) -> None:
        self._append_digit("8")

    def press_digit_9(self) -> None:
        self._append_digit("9")

    def press_decimal(self) -> None:
        self._append_decimal()

    def press_equals(self) -> None:
        if self.display == "Error":
            return

        try:
            if self._pending_root:
                base = self._parse_display()
                if abs(self._root_index) < 1e-12:
                    raise ZeroDivisionError
                result = base ** (1.0 / self._root_index)
                self._pending_root = False
            elif self._expression:
                if not self._fresh_entry:
                    full_expression = self._expression + self.display
                elif self._expression[-1] in "+-*/%":
                    full_expression = self._expression[:-1]
                else:
                    full_expression = self._expression
                result = self._evaluate_expression(full_expression)
            else:
                return

            self._set_display(result)
            self._expression = self.display
        except Exception:
            self.display = "Error"
            self._expression = ""
            self._pending_root = False
            self._fresh_entry = True
