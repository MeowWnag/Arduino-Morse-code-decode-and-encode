// Arduino 摩斯密碼加解密專案
// 使用方法:
// 1.在Serial Monitor的輸入欄輸入英文字母，然後按Enter，蜂鳴器和LED就會按照摩斯密碼輸出聲音與閃光。
// 2.通過按下麵包板上的按鈕輸入你想輸入的摩斯密碼，程式會解碼摩斯密碼並輸出在Serial Monitor上。
// 未識別的序列會發出哔聲並顯示"?"。
// 提示：
//   保持點短，劃長。
//   在字母後留一個短空格。
//   在單詞後留一個較長的空格。
//   嘗試使按壓和釋放乾淨利落。
// 摩斯密碼與英文字母和數字的對照表如下：
// A .-/B -.../C -.-./D -../E ./F ..-./G --./H ..../I ../J .---/K -.-/L .-../M --/N -./O ---/P .--./Q --.-/R .-./S .../T -/U ..-/V ...-/W .--/X -..-/Y -.--/Z --..
// 0 -----/1 .----/2 ..---/3 ...--/4 ....-/5 ...../6 -..../7 --.../8 ---../9 ----.

// 電路連接說明：
// 壓電蜂鳴器或揚聲器連接到針腳2和地線。
// 按鈕連接在針腳8和地線之間。
// LED(+)連接到針腳13，(-)通過220歐姆電阻接地。

// 定義連接到Arduino的元件針腳
int tonePin = 2;// 蜂鳴器連接的腳位
int toneFreq = 1000;// 蜂鳴器的頻率
int ledPin = 13;// LED燈連接的腳位
int buttonPin = 8;// 按鈕連接的腳位
int debounceDelay = 30;// 消抖延遲時間

int dotLength = 240; // 點的長度，基本單位為毫秒
// dotLength = 基本速度單位
// 240代表每分鐘5個單詞的速度
// WPM = 1200/dotLength
// 若要使用其他速度，可使用 dotLength = 1200/(WPM)

// 其他長度由點的長度計算得出
  int dotSpace = dotLength;
  int dashLength = dotLength*3;
  int letterSpace = dotLength*3;
  int wordSpace = dotLength*7; 
  float wpm = 1200./dotLength;
  
int t1, t2, onTime, gap;
bool newLetter, newWord, letterFound, keyboardText;
int lineLength = 0;
int maxLineLength = 20; // 最大行長度

// 字母對應的摩爾斯電碼
char* letters[] = 
{
".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", // A-I
".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", // J-R 
"...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.." // S-Z
};

// 數字對應的摩爾斯電碼
char* numbers[] = 
{
"-----", ".----", "..---", "...--", "....-", //0-4
".....", "-....", "--...", "---..", "----." //5-9
};

String dashSeq = "";// 存儲點和劃的序列
char keyLetter, ch;// 存儲解碼後的字母和讀取的字符
int i, index;// 循環索引和字母索引

void setup() 
{
  delay(500);// 啟動延遲
  pinMode(ledPin, OUTPUT);// 設置LED燈腳位為輸出模式
  pinMode(tonePin, OUTPUT);// 設置蜂鳴器腳位為輸出模式
  pinMode(buttonPin, INPUT_PULLUP);// 設置按鈕腳位為上拉輸入模式
  Serial.begin(9600);// 開始序列通訊，波特率為9600
  // 打印初始化信息
  Serial.println();
  Serial.println("-------------------------------");
  Serial.println("摩爾斯電碼解碼器/編碼器");
  Serial.print("Speed=");
  Serial.print(wpm);
  Serial.print("wpm, ");
  Serial.print("dot=");
  Serial.print(dotLength);
  Serial.println("ms");   
  
// 測試LED燈和蜂鳴器
  tone(tonePin, toneFreq);// 啟動蜂鳴器
  digitalWrite(ledPin, HIGH);// 點亮LED燈
  delay(2000);// 延遲2秒
  digitalWrite(ledPin, LOW);// 熄滅LED燈
  noTone(tonePin);// 關閉蜂鳴器
  delay(600);// 延遲600毫秒

 // 閃爍以示範預期的按鍵速度
  // H
  Serial.print("H .... ");
  index = 'H' - 65;  
  flashSequence(letters[index]);
  delay(wordSpace);
  // E
  Serial.print("E . ");
  index = 'E' - 65;
  flashSequence(letters[index]);  
  delay(letterSpace);
  // L 
  Serial.print("L .-.. ");
  index = 'L' - 65;
  flashSequence(letters[index]);
  delay(letterSpace);
  // L
  Serial.print("L .-.. ");  
  index = 'L' - 65;
  flashSequence(letters[index]);
  delay(letterSpace);
  // O
  Serial.print("O --- ");
  index = 'O' - 65;
  flashSequence(letters[index]);
  delay(wordSpace);
  // W
  Serial.print("W .-- ");
  index = 'W' - 65;
  flashSequence(letters[index]);
  delay(letterSpace);
  // O
  Serial.print("O --- ");
  index = 'O' - 65;
  flashSequence(letters[index]);  
  delay(letterSpace);
  // R
  Serial.print("R .-. ");
  index = 'R' - 65;
  flashSequence(letters[index]);
  delay(letterSpace); 
  // L
  Serial.print("L .-.. ");
  index = 'L' - 65;
  flashSequence(letters[index]);
  delay(letterSpace);
  // D
  Serial.print("D -.. ");
  index = 'D' - 65;
  flashSequence(letters[index]);
  delay(wordSpace);
  //Hello World
  Serial.println("HELLO WORLD，測試完成");

  Serial.println();
  Serial.println("-------------------------------");
  Serial.println("點擊Serial Monitor中的欄位，");
  Serial.println("輸入文本並按Enter，或是");
  Serial.println("通過按鍵輸入摩爾斯電碼進行解碼：");
  Serial.println("-------------------------------");
      
  newLetter = false; // 如果為false，則不檢查字母結束間隔
  newWord = false;  // 如果為false，則不檢查單詞結束間隔
  keyboardText = false; // 如果為false，則不處理鍵盤輸入的文本
}

// 主循環
void loop() 
{
// 檢查是否有鍵盤輸入
  if (Serial.available() > 0)
  {
    // 如果是第一次鍵盤輸入，打印標頭
    if (keyboardText == false) 
    {
      Serial.println();
      Serial.println("-------------------------------");
    }
    keyboardText = true;// 設置為處理鍵盤輸入
    ch = Serial.read();// 讀取一個字符
    // 如果字符是小寫字母，轉換為大寫
    if (ch >= 'a' && ch <= 'z')
    { ch = ch-32; }
    // 如果字符是大寫字母，打印並閃爍對應的摩爾斯電碼
    if (ch >= 'A' && ch <= 'Z')
    {
      Serial.print(ch); 
      Serial.print(" ");
      Serial.println(letters[ch-'A']);
      flashSequence(letters[ch-'A']);
      delay(letterSpace);
    }
    // 如果字符是數字，打印並閃爍對應的摩爾斯電碼
    if (ch >= '0' && ch <= '9')
    {
      Serial.print(ch);
      Serial.print(" ");
      Serial.println(numbers[ch-'0']);
      flashSequence(numbers[ch-'0']);
      delay(letterSpace);
    }
    // 如果字符是空格，打印並延遲單詞間隔
    if (ch == ' ')
    {
      Serial.println("_");
      delay(wordSpace);    
    } 

// 如果鍵盤輸入結束，打印標頭   
     if (Serial.available() <= 0) 
     {
      Serial.println();
      Serial.println("輸入文本或通過按鍵輸入：");
      Serial.println("-------------------------------");
      keyboardText = false;// 重置鍵盤輸入標誌
     }
  }

 // 檢查按鈕是否被按下
  if (digitalRead(buttonPin) == LOW ) // 按鈕被按下
  {
    newLetter = true; 
    newWord = true;
    t1=millis(); // 記錄按鈕按下的時間
    digitalWrite(ledPin, HIGH); // 點亮LED燈並發出聲音
    tone(tonePin, toneFreq);
    delay(debounceDelay);     // 消抖延遲
    while (digitalRead(buttonPin) == LOW ) // 等待按鈕釋放
      {delay(debounceDelay);}
      delay(debounceDelay);
       // 按鈕釋放後的處理
    t2 = millis();  // 記錄按鈕釋放的時間
    onTime=t2-t1;  // 計算按鈕按下的持續時間
    digitalWrite(ledPin, LOW); // 熄滅LED燈並關閉聲音
    noTone(tonePin); 
    
// 判斷是點還是劃

    if (onTime <= dotLength*1.5) // 允許時間長達50%以上
      {
        dashSeq = dashSeq + ".";// 如果是點，添加到序列中
        } 
    else 
      {
        dashSeq = dashSeq + "-";// 如果是劃，添加到序列中
        }
  }  // 結束按鈕按下的處理
  
// 檢查是否有足夠的間隔來表示字母的結束
  gap=millis()-t2; // 計算按鈕釋放後的間隔時間
  if (newLetter == true && gap>=letterSpace)  
  { 
// 通過摩爾斯電碼序列來尋找匹配的字母
    letterFound = false; 
    keyLetter = 63; // 如果找不到匹配，則使用"?"
    for (i=0; i<=25; i++)
    {
      if (dashSeq == letters[i]) 
      {
        keyLetter = i+65;// 找到匹配的字母
        letterFound = true;   
        break ;    // 如果找到字母，則停止檢查 
      }
    }
    // 如果沒有找到字母，則檢查數字
    if(letterFound == false) 
    {
      for (i=0; i<=10; i++)
      {
      if (dashSeq == numbers[i]) 
        {
          keyLetter = i+48;// 找到匹配的數字
          letterFound = true;   
          break ;    // 如果找到數字，則停止檢查
        }
      }
    }    
    Serial.print(keyLetter);// 打印找到的字母或數字
    if(letterFound == false) // 如果序列未知，則發出警告聲
    {
      tone(tonePin, 100, 500);
    }  
    newLetter = false; // 重置新字母標誌
    dashSeq = "";
    lineLength=lineLength+1;// 增加行長度計數
  }  
// 如果已經識別並打印了鍵入的字母
// 如果間隔時間大於等於單詞間隔，則在單詞之間插入空格
  if (newWord == true && gap>=wordSpace*1.5)
    { 
     newWord = false; 
     Serial.print("_");  // 打印空格以分隔單詞
     lineLength=lineLength+1;// 增加行長度計數
     
// 閃爍LED燈來表示新單詞的開始
    digitalWrite(ledPin, HIGH);
    delay(25);
    digitalWrite(ledPin, LOW);       
    } 

// 如果行長度達到最大值，則插入換行符
  if (lineLength >= maxLineLength) 
    {
      Serial.println();// 換行
      lineLength = 0;// 重置行長度計數
    }      
} 

// 閃爍序列的函數，用於閃爍點和劃
void flashSequence(char* sequence)
{
  int i = 0;
  while (sequence[i] == '.' || sequence[i] == '-')
  {
    flashDotOrDash(sequence[i]);// 閃爍每個點或劃
    i++;
  }
}

// 閃爍點或劃的函數
void flashDotOrDash(char dotOrDash)
{
  digitalWrite(ledPin, HIGH);// 點亮LED燈
  tone(tonePin, toneFreq); // 發出聲音
  if (dotOrDash == '.')
   { 
    delay(dotLength); // 如果是點，則延遲點的長度
    }
     else
   { 
    delay(dashLength); // 如果是劃，則延遲劃的長度
    }

  digitalWrite(ledPin, LOW);// 熄滅LED燈
  noTone(tonePin);// 關閉聲音
  delay(dotLength); // 延遲點的長度
}