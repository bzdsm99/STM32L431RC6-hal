
/*************************************************
函数名：lcd_fill
功能：lcd区域填充
入口参数：xy起点和终点，填充颜色
返回值：无
*************************************************/
void lcd_fill(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color)
{
    unsigned int i, m;
    Lcd_SetRegion(xs, ys, xe, ye); // 设置填充区域
    Lcd_WriteIndex(0x2C); // 写入数据指令
    for (i = xs; i <= xe; i++) {
        for (m = ys; m <= ye; m++) {    
            LCD_WriteData_16Bit(color); // 填充指定颜色
        }   
    }
}
