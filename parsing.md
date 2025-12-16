時刻変更文字列の構文解析
========================

GNU coureutils に含まれる bison の定義ファイルには時刻変更の文字列から `timespec` 構造体の値に変換する処理のソースコードとともに、字句解析のトークンや [yylex](./yylex.md) 関数、構文解析の文法が記述されています。文字列は文法に従って日時の各パラメータに解析され、その値によって時刻が変更されます。

時刻変更の文字列を BNF 形式で表すと以下の通りです。文字列の全体は &lt;spec&gt; で、&lt;timespec&gt; か &lt;items&gt;（[&lt;zone&gt;](#zone)、[&lt;local_zone&gt;](#local_zone)、[&lt;datetime&gt;](#datetime)、[&lt;time&gt;](#time)、[&lt;date&gt;](#date)、[&lt;day&gt;](#day)、[&lt;rel&gt;](#rel)、[&lt;number&gt;](#number) の集まり）で表されます。なお、空白文字のみの文字列は解析前に `"0"` に置換されます。

**文法**

```
  <spec> ::= <timespec> | <items>

  <items> ::= <items> <item>

  <item> ::= <zone> | <local_zone> | <datetime> | <date> | <time> | <day> | <rel> | <number>
```

構文解析で取得された日時のパラメータは月、日、時、分、秒の値が正しくない場合、タイムゾーンでの UTC オフセットが `-24` から `24` 時間までの範囲にない場合、加算や乗算で値がオーバーフローした場合はエラーになります。また、相対的な日時以外のパラメータが２回以上取得された場合はエラーになり、&lt;zone&gt; と &lt;local_zone&gt; は同じ文字列内に含められません。

日付では取得された年の値が `0` 以上 `68` 以下の場合は `2000`、 `69` 以上 `99` 以下の場合は `1900` が足されます。時刻では午後を示す値が取得された場合は時間の値に `12` が足され、分以下で指定されない値は `0` が設定されます。

<a id="timespec"></a>
### &lt;timespec&gt;

&lt;timespec&gt; は `'@'` の後ろに[整数](./yylex.md#number)や[小数](./yylex.md#decimal_number)を指定する文字列で表されます。

**文法**

```
  <timespec> ::= '@' <seconds>

  <seconds> ::= <signed_seconds> | <unsigned_seconds>;

  <signed_seconds> ::= tSDECIMAL_NUMBER | tSNUMBER

  <unsigned_seconds> ::= tUDECIMAL_NUMBER | tUNUMBER
```

文字列が &lt;timespec&gt; と解釈されると、整数（部）が 1970-01-01 00:00 UTC からの秒、小数が指定された場合は小数部がナノ秒（Windows では 100 ナノ秒）の値として取得されます。これらの値には変更する時刻の計算が行われません。

<a id="local_zone"></a>
### &lt;local_zone&gt;

&lt;local_zone&gt; は[タイムゾーン](./yylex.md#zone)での標準時や夏時間の略称（Windows では[ローカルタイムゾーン](./yylex.md#local_zone)名）、または、その後ろに [`"DST"`](./yylex.md#dst) を指定する文字列で表されます。

**文法**

```
  <local_zone> ::= tLOCAL_ZONE
                 | tLOCAL_ZONE tDST
```

文字列が &lt;local_zone&gt; と解釈されると、ローカルタイムゾーンで夏時間の影響を受けるかどうかの値が取得され、"DST" が指定された場合は（夏時間の影響を受ける）`1` に設定されます。この値が現在時刻の夏時間の影響を受けるかどうかの値と異なる場合、変更する時刻の計算でエラーになります。

<a id="zone"></a>
### &lt;zone&gt;

&lt;zone&gt; は[タイムゾーン](./yylex.md#zone)での標準時や夏時間の略称、軍事タイムゾーンのアルファベット、または、夏時間の略称、`'T'` 以外の後ろに [`"DST"`](./yylex.md#dst)、`"+hhmm"`、`"-hhmm"`、`"+hh"`、`"-hh"`、`"+hh:mm"`、`"-hh:mm"` を指定する文字列で表されます。

**文法**

```
  <zone> ::= tZONE
           | tZONE tDST
           | tZONE tSNUMBER
           | tZONE tSNUMBER ':' tUNUMBER
           | tDAYZONE
```

文字列が &lt;zone&gt; と解釈されると、タイムゾーンでの UTC オフセットが取得されます。後ろに "DST"  が指定された場合はその値に１時間が足され、`"+hhmm"`、`"-hhmm"` 等が指定された場合は UTC オフセットとして加算されます。

<a id="datetime"></a>
### &lt;datetime&gt;

&lt;datetime&gt; は ISO 8601 フォーマットの日時であり、&lt;iso_8601_date&gt; `'T'` &lt;iso_8601_time&gt; で表されます。&lt;iso_8601_date&gt; は `"YYYY-MM-DD"` を指定する文字列で表されます。&lt;iso_8601_time&gt; は `"hh:mm:ss.nnnnnnnnn"`（Windows では `"hh:mm:ss.nnnnnnn"`）、`"hh:mm:ss"`、`"hh:mm"` を指定する文字列、または、それらの文字列や `"hh"` の後ろに `"+hhmm"`、`"-hhmm"`、`"+hh"`、`"-hh"`、`"+hh:mm"`、`"-hh:mm"` を指定する文字列で表されます。

**文法**

```
  <datetime> ::= <iso_8601_date> 'T' <iso_8601_time>

  <iso_8601_date> ::= tUNUMBER tSNUMBER tSNUMBER

  <iso_8601_time> ::= tUNUMBER tSNUMBER
                    | tUNUMBER tSNUMBER ':' tUNUMBER
                    | tUNUMBER ':' tUNUMBER
                    | tUNUMBER ':' tUNUMBER tSNUMBER
                    | tUNUMBER ':' tUNUMBER tSNUMBER ':' tUNUMBER
                    | tUNUMBER ':' tUNUMBER ':' <unsigned_seconds>
                    | tUNUMBER ':' tUNUMBER ':' <unsigned_seconds> tSNUMBER
                    | tUNUMBER ':' tUNUMBER ':' <unsigned_seconds> tSNUMBER ':' tUNUMBER
```

文字列が &lt;datetime&gt; と解釈されると、`"YYYY"` が年、`"MM"` が月、`"DD"` が日、`"hh"` が時、`"mm"` が分、`"ss"` が秒、`"nnnnnnnnn"` がナノ秒（Windows では `"nnnnnnn"` が 100 ナノ秒）の値として取得され、`"+hhmm"`、`"-hhmm"` 等が UTC オフセットとして計算されます。なお、`"YYYY+MM+DD"` を指定した場合は月、日に負の値が取得されますが、変更する時刻の計算でエラーになります。

<a id="date"></a>
### &lt;date&gt;

&lt;date&gt; は `"MM/DD"` や `"YYYY/MM/DD"` を指定する文字列、[月名](./yylex.md#month_day)の後ろに `"DD"`、`"DD,YYYY"`、`"-DD-YYYY"` を指定する文字列、`"DD"` の後ろに月名やその後ろに `"YYYY"`、`"-YYYY"` を指定する文字列、または、[&lt;iso_8601_date&gt;](#datetime) で表されます。

**文法**

```
  <date> ::= tUNUMBER '/' tUNUMBER
           | tUNUMBER '/' tUNUMBER '/' tUNUMBER
           | tMONTH tUNUMBER
           | tMONTH tUNUMBER ',' tUNUMBER
           | tMONTH tSNUMBER tSNUMBER
           | tUNUMBER tMONTH
           | tUNUMBER tMONTH tUNUMBER
           | tUNUMBER tMONTH tSNUMBER
           | <iso_8601_date>
```

文字列が &lt;date&gt; と解釈されると、`"YYYY"` が年、`"MM"` が月、`"DD"` が日の値として取得されます。

<a id="time"></a>
### &lt;time&gt;

&lt;time&gt; は `"hh:mm:ss.nnnnnnnnn"`（Windows では `"hh:mm:ss.nnnnnnn"`）、`"hh:mm:ss"`、`"hh:mm"`、`"hh"` の後ろに[午前・午後](./yylex.md#meridian)を指定する文字列、または、[&lt;iso_8601_time&gt;](#datetime) で表されます。

**文法**

```
  <time> ::= tUNUMBER tMERIDIAN
           | tUNUMBER ':' tUNUMBER tMERIDIAN
           | tUNUMBER ':' tUNUMBER ':' <unsigned_seconds> tMERIDIAN
           | <iso_8601_time>
```

文字列が &lt;time&gt; と解釈されると、`"hh"` が時、`"mm"` が分、`"ss"` が秒、`"nnnnnnnnn"` がナノ秒（Windows では `"nnnnnnn"` が 100 ナノ秒）の値として取得されます。

<a id="day"></a>
### &lt;day&gt;

&lt;day&gt; は[曜日名](./yylex.md#month_day)、その後ろに `','`、または、曜日名の前に[序数詞](./yylex.md#ordinal)、[整数](./yylex.md#number)を指定する文字列で表されます。

**文法**

```
  <day> ::= tDAY
          | tDAY ','
          | tORDINAL tDAY
          | tUNUMBER tDAY
```

文字列が &lt;day&gt; と解釈されると、曜日の（日曜日を `0` として数える）値として取得され、序数詞に関連付けられたり、整数に表されたりする値は曜日が何週先かに設定されます。

<a id="rel"></a>
### &lt;rel&gt;

&lt;rel&gt; は[相対的な日時](./yylex.md#rel_unit)であり、年、月、日、時、分、秒などの英単語、その前に[序数詞](./yylex.md#ordinal)や[整数](./yylex.md#number)を指定する文字列、`"second"`、`"seconds"`、`"sec"`、`"secs"` の前に[小数](./yylex.md#decimal_number)を指定する文字列、または、それらの後ろに[以前・以後](./yylex.md#ago)を指定する文字列（`"tomorrow"`、`"yesterday"`、`"today"`、`"now"` は単独）で表されます。

**文法**

```
  <rel> ::= <relunit> tAGO
          | <relunit>
          | tDAY_SHIFT

  <relunit> ::= tYEAR_UNIT
              | tORDINAL tYEAR_UNIT
              | tUNUMBER tYEAR_UNIT
              | tSNUMBER tYEAR_UNIT
              | tMONTH_UNIT
              | tORDINAL tMONTH_UNIT
              | tUNUMBER tMONTH_UNIT
              | tSNUMBER tMONTH_UNIT
              | tDAY_UNIT
              | tORDINAL tDAY_UNIT
              | tUNUMBER tDAY_UNIT
              | tSNUMBER tDAY_UNIT
              | tHOUR_UNIT
              | tORDINAL tHOUR_UNIT
              | tUNUMBER tHOUR_UNIT
              | tSNUMBER tHOUR_UNIT
              | tMINUTE_UNIT
              | tORDINAL tMINUTE_UNIT
              | tUNUMBER tMINUTE_UNIT
              | tSNUMBER tMINUTE_UNIT
              | tSEC_UNIT
              | tORDINAL tSEC_UNIT
              | tUNUMBER tSEC_UNIT
              | tSNUMBER tSEC_UNIT
              | tSDECIMAL_NUMBER tSEC_UNIT
              | tUDECIMAL_NUMBER tSEC_UNIT
```

文字列が &lt;rel&gt; と解釈されると、相対的な日時に関連付けられた値が取得され、序数詞に関連付けられたり、整数に表されたりする値が掛けられ、後ろに `"ago"` が指定された場合はさらに `-1` が掛けられます。

<a id="number"></a>
### &lt;number&gt;

&lt;number&gt; は[符号なし整数](./yylex.md#number)で表されます。前に年や[相対的な日時](./yylex.md#rel_unit)が指定されずに月と日が指定され、前に時刻が指定されるか整数が３桁以上の場合は `"YYYY"`、そうでない場合、整数が５桁以上は `"YYYYMMDD"`、４、２桁以下は `"hhmm"`、`"hh"` を指定する文字列で表されます。

**文法**

```
  <number> ::= tUNUMBER
```

文字列が &lt;number&gt; と解釈されると、`"YYYY"` が 年、`"MM"` が 月、`"DD"` が 日、`"hh"` が時、`"mm"` が分の値として取得されます。
