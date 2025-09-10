#include <iostream>
#include <cmath>
#include <limits>

using namespace std;

int main()
{
    double a, b, c;

    cout << "Введите коэффициенты квадратного уравнения (a, b, c): ";

    // Проверка корректности ввода
    while (!(cin >> a >> b >> c))
    {
        cout << "Ошибка ввода! Пожалуйста, введите три числа: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    // Обработка вырожденного случая (a = 0)
    if (a == 0)
    {
        // Линейное уравнение bx + c = 0
        if (b == 0)
        {
            if (c == 0)
            {
                cout << "x - любое число" << endl;
            }
            else
            {
                cout << "Нет решений" << endl;
            }
        }
        else
        {
            double x = -c / b;
            cout << "x = " << x << endl;
        }
        return 0;
    }

    // Вычисление дискриминанта
    double discriminant = b * b - 4 * a * c;

    // Обработка случаев для квадратного уравнения
    if (discriminant > 0)
    {
        double x1 = (-b + sqrt(discriminant)) / (2 * a);
        double x2 = (-b - sqrt(discriminant)) / (2 * a);
        cout << "x1 = " << x1 << endl;
        cout << "x2 = " << x2 << endl;
    }
    else if (discriminant == 0)
    {
        double x = -b / (2 * a);
        cout << "x = " << x << endl;
    }
    else
    {
        // Комплексные корни
        double real_part = -b / (2 * a);
        double imaginary_part = sqrt(-discriminant) / (2 * a);
        cout << "x1 = " << real_part << " + " << imaginary_part << "i" << endl;
        cout << "x2 = " << real_part << " - " << imaginary_part << "i" << endl;
    }

    return 0;
}