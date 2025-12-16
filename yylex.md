yylex
=====

yylex 関数は渡された文字列の先頭から字句を解析し、判別したトークンに対してマクロで定義された値を返し、どれにも当てはまらない場合は `'-'` や `'+'` をスキップして記号１文字をトークンとして返し、それ以外はエラーとして `'?'` を返します。トークンの区切りは空白、`'\t'`、`'\n'`、`'\r'` が使用され、アルファベットの大文字・小文字は区別されません。また、左右のパーレンで囲まれた部分は解析せずスキップします。

yylex 関数ではタイムゾーン、ローカルタイムゾーン、夏時間、月名・曜日名、相対的な日時、以前・以後、午前・午後、序数、整数、小数のトークンを判別できます。これらが数値を表したり、数値に関連付けられたりする場合、その値がポインタで渡された共用体に取得されます。

<a id="zone"></a>
### タイムゾーン

標準時、夏時間の略称は `tZONE`、`tDAYZONE` と判別され、タイムゾーンでの UTC オフセットに関連付けられます。また、`'A'` から `'Z'` のアルファベット１文字は[軍事タイムゾーン](https://en.wikipedia.org/wiki/List_of_military_time_zones)（英語版 Wikipedia を参照）の `tZONE` と判別されますが、`'J'` はローカルタイムを示すため UTC オフセットに関連付けられません。

**タイムゾーンの略称や名称**

  | 略称 | 名称                         | トークン | UTC オフセット |
  | ---- | ---------------------------- | -------- | -------------- |
  | UTC  | 協定世界時                   | tZONE    |   0 時間       |
  | UT   | （協定）世界時               | tZONE    |   0 時間       |
  | GMT  | グリニッジ標準時             | tZONE    |   0 時間       |
  | BST  | 英国夏時間                   | tDAYZONE |   0 時間       |
  | WET  | 西ヨーロッパ時間             | tZONE    |   0 時間       |
  | WEST | 西ヨーロッパ夏時間           | tDAYZONE |   0 時間       |
  | ART  | アルゼンチン時間             | tZONE    | - 3 時間       |
  | BRT  | ブラジル時間                 | tZONE    | - 3 時間       |
  | BRST | ブラジル夏時間               | tDAYZONE | - 3 時間       |
  | NST  | ニューファンドランド標準時   | tZONE    | - 3 時間 30 分 |
  | NDT  | ニューファンドランド夏時間   | tDAYZONE | - 3 時間 30 分 |
  | AST  | 大西洋標準時                 | tZONE    | - 4 時間       |
  | ADT  | 大西洋夏時間                 | tDAYZONE | - 4 時間       |
  | CLT  | チリ時間                     | tZONE    | - 4 時間       |
  | CLST | チリ夏時間                   | tDAYZONE | - 4 時間       |
  | EST  | （アメリカ）東部標準時       | tZONE    | - 5 時間       |
  | EDT  | （アメリカ）東部夏時間       | tDAYZONE | - 5 時間       |
  | CST  | （アメリカ）中部標準時       | tZONE    | - 6 時間       |
  | CDT  | （アメリカ）中部夏時間       | tDAYZONE | - 6 時間       |
  | MST  | （アメリカ）山岳部標準時     | tZONE    | - 7 時間       |
  | MDT  | （アメリカ）山岳部夏時間     | tDAYZONE | - 7 時間       |
  | PST  | （アメリカ）太平洋標準時     | tZONE    | - 8 時間       |
  | PDT  | （アメリカ）太平洋夏時間     | tDAYZONE | - 8 時間       |
  | AKST | アラスカ標準時               | tZONE    | - 9 時間       |
  | AKDT | アラスカ夏時間               | tDAYZONE | - 9 時間       |
  | HST  | ハワイ標準時                 | tZONE    | -10 時間       |
  | HAST | ハワイ・アリューシャン標準時 | tZONE    | -10 時間       |
  | HADT | ハワイ・アリューシャン夏時間 | tDAYZONE | -10 時間       |
  | SST  | サモア標準時                 | tZONE    | -12 時間       |
  | WAT  | 西アフリカ時間               | tZONE    |   1 時間       |
  | CET  | 中央ヨーロッパ時間           | tZONE    |   1 時間       |
  | CEST | 中央ヨーロッパ夏時間         | tDAYZONE |   1 時間       |
  | MET  | 中部ヨーロッパ時間           | tZONE    |   1 時間       |
  | MEST | 中部ヨーロッパ夏時間         | tDAYZONE |   1 時間       |
  | MEZ  | 中部ヨーロッパ時間           | tZONE    |   1 時間       |
  | MESZ | 中部ヨーロッパ夏時間         | tDAYZONE |   1 時間       |
  | EET  | 東ヨーロッパ時間             | tZONE    |   2 時間       |
  | EEST | 東ヨーロッパ夏時間           | tDAYZONE |   2 時間       |
  | CAT  | 中央アフリカ時間             | tZONE    |   2 時間       |
  | SAST | 南アフリカ標準時             | tZONE    |   2 時間       |
  | EAT  | 東アフリカ時間               | tZONE    |   3 時間       |
  | MSK  | モスクワ時間                 | tZONE    |   3 時間       |
  | MSD  | モスクワ夏時間               | tDAYZONE |   3 時間       |
  | IST  | インド標準時                 | tZONE    |   5 時間 30 分 |
  | SGT  | シンガポール時間             | tZONE    |   8 時間       |
  | KST  | 韓国標準時                   | tZONE    |   9 時間       |
  | JST  | 日本標準時                   | tZONE    |   9 時間       |
  | GST  | グアム標準時                 | tZONE    |  10 時間       |
  | NZST | ニュージーランド標準時       | tZONE    |  12 時間       |
  | NZDT | ニュージーランド夏時間       | tDAYZONE |  12 時間       |

※同じタイムゾーンで標準時と夏時間は同じ UTC オフセットに関連付けられます。しかし、`tDAYZONE` の場合は構文解析で値に `1` 時間が足されます。

<a id="local_zone"></a>
### ローカルタイムゾーン

POSIX の strftime 関数のフォーマットに `%Z` を指定して呼び出すと、GLIBC では[標準時や夏時間の略称](#zone)、Windows では［日付と時刻］で設定されたタイムゾーンの名称を得られます。yylex 関数がこの略称や名称を検出すると `tLOCAL_ZONE` と判別され、タイムゾーンでの夏時間の影響を受けるかどうか（[tm 構造体の tm_isdst メンバ](./mktime.md#spec)）の値に関連付けられます。

なお、Windows のローカルタイムゾーン名を指定する場合は名称から `"GMT標準時"`、`"GMT夏時間"` と空白を削除し、`"台北"`、`"東京"` とパーレンで囲まれた部分を省略してください。下表では GLIBC の GMT に対するタイムゾーンが２つありますが、ロンドンは GMT 標準時に含まれます。

**主なローカルタイムゾーン**

  | GLIBC | Windows                      |
  | ----- | ---------------------------- |
  | UTC   | 協定世界時                   |
  | GMT   | グリニッジ標準時             |
  | GMT   | GMT 標準時                   |
  | BST   | GMT 夏時間                   |
  | BRT   | 南アメリカ東部標準時         |
  | AST   | 大西洋標準時                 |
  | ADT   | 大西洋夏時間                 |
  | EST   | 東部標準時                   |
  | EDT   | 東部夏時間                   |
  | CST   | 中部標準時                   |
  | CDT   | 中部夏時間                   |
  | MST   | 山地標準時                   |
  | MDT   | 山地夏時間                   |
  | PST   | 太平洋標準時                 |
  | PDT   | 太平洋夏時間                 |
  | AKST  | アラスカ標準時               |
  | AKDT  | アラスカ夏時間               |
  | HST   | ハワイ標準時                 |
  | CET   | 西ヨーロッパ標準時           |
  | CEST  | 西ヨーロッパ夏時間           |
  | CET   | 中央ヨーロッパ標準時         |
  | CEST  | 中央ヨーロッパ夏時間         |
  | CET   | ロマンス標準時               |
  | CEST  | ロマンス夏時間               |
  | CET   | 中央ヨーロピアン標準時       |
  | CEST  | 中央ヨーロピアン夏時間       |
  | WAT   | 西中央アフリカ標準時         |
  | EET   | 東ヨーロッパ標準時           |
  | EEST  | 東ヨーロッパ夏時間           |
  | SAST  | 南アフリカ標準時             |
  | EAT   | 東アフリカ標準時             |
  | IST   | インド標準時                 |
  | ICT   | 東南アジア標準時             |
  | SGT   | マレー半島標準時             |
  | CST   | 台北 (標準時)                |
  | KST   | 韓国 (標準時)                |
  | JST   | 東京 (標準時)                |
  | PGT   | 西太平洋 (標準時)            |
  | NZST  | ニュージーランド標準時       |
  | NZDT  | ニュージーランド夏時間       |

※ MSVCRT で指定可能なタイムゾーン名については [既定のタイムゾーン](https://learn.microsoft.com/ja-jp/windows-hardware/manufacture/desktop/default-time-zones#time-zones) を参照

<a id="dst"></a>
### 夏時間

Daylight Saving Time（夏時間）の略 `"DST"` は `tDST` と判別され、前に [`tZONE`](#zone) を伴う場合はタイムゾーンでの UTC オフセットに `1` 時間が足され、[`tLOCAL_ZONE`](#local_zone) を伴う場合は夏時間の影響を受けるかどうかの値に（夏時間の影響を受ける）`1` が設定されます。

<a id="month_day"></a>
### 月名・曜日名

月名、曜日名の英単語は `tMONTH`、`tDAY` と判別され、それぞれ月の数字、曜日の（日曜日を `0` として数える）値に関連付けられます。最初のアルファベット３文字やそれにピリオドを付けた短縮形、９月は `"Sept"`、火曜日は `"Tues"`、水曜日は `"Wednes"`、木曜日は `"Thur"` や `"Thurs"` もトークンに指定できます。

<a id="rel_unit"></a>
### 相対的な日時

英単語の `"year"`、`"month"`、`"day"`、`"hour"`、`"minute"`、`"second"` は `tYEAR_UNIT`、`tMONTH_UNIT`、`tDAY_UNIT`、`tHOUR_UNIT`、`tMINUTE_UNIT`、`tSEC_UNIT` と判別され、相対的な `1` 年、`1` 月、`1` 日、`1` 時、`1` 分、`1` 秒を表します。後ろに `"s"` を付けた複数形、分は `"min"`、秒は `"sec"` もトークンに指定できます。`"day"` 以外にも下表の英単語が `tDAY_SHIFT` や `tDAY_UNIT` と判別され、相対的な日に関連付けられます。

**"day" 以外の日を表す英単語**

  |           | トークン   | 相対日 |
  | --------- | ---------- | -----: |
  | yesterday | tDAY_SHIFT |  -1 日 |
  | today     | tDAY_SHIFT |   0 日 |
  | now       | tDAY_SHIFT |   0 日 |
  | tomorrow  | tDAY_SHIFT |   1 日 |
  | week      | tDAY_UNIT  |   7 日 |
  | fortnight | tDAY_UNIT  |  14 日 |

<a id="ago"></a>
### 以前・以後

英単語の `"ago"`、`"hence"` は `tAGO` と判別され、以前、以後の日時に変更するため掛ける `-1`、`1` の値に関連付けられます。

<a id="meridian"></a>
### 午前・午後

英単語の `"AM"` や `"A.M."`、`"PM"` や `"P.M."` は `tMERIDIAN` と判別され、午前、午後を示す値に関連付けられます。

<a id="ordinal"></a>
### 序数詞

下表の英単語は `tORDINAL` と判別され、値に関連付けられます。後ろに伴う [`tDAY`](#month_day) が何週先かの値を表したり、[`tYEAR_UNIT`](#rel_unit)、[`tMONTH_UNIT`](#rel_unit)、[`tDAY_UNIT`](#rel_unit)、[`tHOUR_UNIT`](#rel_unit)、[`tMINUTE_UNIT`](#rel_unit)、[`tSEC_UNIT`](#rel_unit) に関連付けられた相対的な日時の値に掛られたりします。

**序数の英単語**

  |          | 値 |
  | -------- | -- |
  | last     | -1 |
  | this     |  0 |
  | next     |  1 |
  | first    |  1 |
  | third    |  3 |
  | fourth   |  4 |
  | fifth    |  5 |
  | sixth    |  6 |
  | seventh  |  7 |
  | eighth   |  8 |
  | ninth    |  9 |
  | tenth    | 10 |
  | eleventh | 11 |
  | twelfth  | 12 |

<a id="number"></a>
### 整数

符号付きの整数、符号なしの整数は `tSNUMBER`、`tUNUMBER` と判別され、タイムゾーンでの UTC オフセット、年、月、日、時、分、秒を表したり、後ろに伴う [`tDAY`](#month_day) が何週先かの値を表したり、後ろに伴う [`tYEAR_UNIT`](#rel_unit)、[`tMONTH_UNIT`](#rel_unit)、[`tDAY_UNIT`](#rel_unit)、[`tHOUR_UNIT`](#rel_unit)、[`tMINUTE_UNIT`](#rel_unit)、[`tSEC_UNIT`](#rel_unit) に関連付けられた相対的な日時の値に掛られたりします。

<a id="decimal_number"></a>
### 小数

符号付きの小数、符号なしの小数は `tSDECIMAL_NUMBER`、`tUDECIMAL_NUMBER` と判別され、整数部は秒、小数部はナノ秒を表します。整数部と小数部の区切りには `.` や `,` を指定できます。なお、Windows の小数部は 100 ナノ秒単位です。
