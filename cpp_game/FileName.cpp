#include <SFML/Graphics.hpp>
#include <iostream>

int main()
{
    // 1. 创建一个 800x600 的窗口
    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "SFML 3.1.0 test passed！");
    window.setFramerateLimit(60); // 限制帧率为 60 FPS

    // 2. 创建一个绿色的圆形，半径 50 像素
    sf::CircleShape circle(50.f);
    circle.setFillColor(sf::Color::Green);
    // 设置初始位置（相对于窗口左上角）
    circle.setPosition({ 100.f, 100.f });

    // 3. 设置移动速度（每帧偏移量）
    float dx = 2.5f;
    float dy = 2.5f;

    // 4. 主循环
    while (window.isOpen())
    {
        // --- 事件处理 ---
        while (const std::optional<sf::Event> event = window.pollEvent())
        {
            // 点击窗口右上角的 'X' 时关闭窗口
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        // --- 更新逻辑（让圆在窗口内弹跳） ---
        sf::Vector2f pos = circle.getPosition();
        pos.x += dx;
        pos.y += dy;

        // 边界碰撞检测（窗口宽800，高600，圆的直径是 100）
        if (pos.x <= 0 || pos.x + 100 >= 800)
            dx = -dx; // 水平反向
        if (pos.y <= 0 || pos.y + 100 >= 600)
            dy = -dy; // 垂直反向

        circle.setPosition(pos);

        // --- 绘图 ---
        window.clear(sf::Color::Black);   // 清空为黑色背景
        window.draw(circle);              // 绘制绿色圆形 aaaa
        window.display();                 // 刷新窗口显示 ABCD
    }

    return 0;
}