= ｱｯ!この分岐「深い」ッ！！

== まえおき

読者も分岐が異常に複雑でネストが深いコードを目撃したことがあるのではないだろうか？
コントロールフローの複雑なコードはダメなコードの典型例と言っても過言ではないであろう。
世の中のコードにはこういうものがあふれているのではないかとすら思う。

複雑な処理を行わなければいけない場合、コードが複雑になるのは仕方がない。
しかし、あまりにも多くの場合分けを一つの関数に押し込めるのは間違いだと思う。

この章では、複雑な分岐をいかに簡潔にコーディングし、関数をどう分離するのかについて議論する。

== 分岐乱舞

一生通ることのない分岐を書くプログラマがいる。

典型例は

 * 非負整数の負数チェック
 * すでにNull検査が済んでいるポインタのダブルチェック

などである。

//emlist[非負整数の負数チェック][cpp]{
// sizeofはstd::size_tを返すので絶対に真になることはない
if constexpr (sizeof(foo) < 0) {
  // ...
}
//}

//emlist[すでにNull検査が済んでいるポインタのダブルチェック][cpp]{
int* ptr
assert(ptr); // 一回目のチェック
if (ptr) { // assertするかハンドルするかどっちかにしろ
  // ...
} else {
  // ...
}
//}


非負整数の負数チェックの場合は修正が簡単で、単に取り除けばいい。

ポインタのダブルチェックの場合、assertするのが正しいのかハンドリングするのがわからない。
そのため、git blameを行い書いた人を問い詰めるか、自分でコードを読んで考えるしかない。

== 正常系と異常系

正常系と異常系が分けて書かれていないというのはコードのわかりにくさの原因のひとつになる。
先に異常系のチェックを行い異常値を返し、以降は正常系の処理を行うというのはよく知られた方法である。

新人が最初に書いた最初のプルリクエストでswitchとifを駆使したコードを見せてくれた。
残念ながら、紙面を圧迫するほど複雑なので疑似コードの掲載を断念せざるを得ない。

この問題へのより良い解答は、モナドを導入することだと思われる。
以下はRustのコードである。
コマンドの入力をパースして数字であれば2倍にして返す関数を書いてみる。
match式を使って分岐をする関数とResultのモナド機能を駆使して書いた関数の2つを見比べてほしい。

//emlist[match式で書いたコマンドラインのパース][rust]{
fn twice_arg(mut argv: env::Args) -> Result<i32, String> {
    match argv.nth(1) {
        None => Err("数字を１つ指定してください。".to_owned()),
        Some(arg1) => {
            match arg1.parse::<i32>() {
                Ok(n) => Ok(2 * n),
                Err(err) => Err(err.to_string()),
            }
        }
    }
}
//}

//emlist[Resultのモナド機能を駆使して書いたコマンドラインのパース][rust]{
fn twice_arg(mut argv: env::Args) -> Result<i32, CliError> {
    argv.nth(1)
        .ok_or(CliError::NotEnoughArgs)?
        .parse::<i32>()
        .map(|x| x * 2)
}
//}

後者の関数がエラー処理を書いてないのがおわかりですか？
Rustの詳細な解説は省きますが、モナディック関数を使うとエラー処理をわざわざ全部書かなくて済みます。
エラーを合成するのです！

ok_or や map のように Option や Result を合成する関数はコンビネータと呼ばれます。
コンビネータ指向でプログラミングをすれば、エラー処理は簡潔になり得ます。

残念ながらC++にこのような高級な機能やライブラリは備わっていないのです。
したがって、モナドライブラリを作ればよいのです。

テンプレートライブラリを書く時間がない？
しかし、コンビネータは「失敗する可能性がある単位」で関数を分離するという重要な示唆を与えてくれます。
すくなくとも、エラーが起こる単位で関数に切り出すことを心がけたいところです。

== この章のまとめ

モナドがない言語が何をやってもダメ。
モナディック関数を作れ。

何種類ものエラーが起こるような関数を書かない。
エラーが起こる最小単位くらいで関数を切り分ける。
