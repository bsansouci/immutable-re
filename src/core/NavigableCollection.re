/**
 * Copyright (c) 2017 - present Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

type t 'a =
  | Empty
  | Instance
      'collection
      (SequentialCollection.s 'collection 'a)
      (SequentialCollection.s 'collection 'a): t 'a;

type navigableCollection 'a = t 'a;

module type S = {
  type a;
  type t;

  include SequentialCollection.S with type a := a and type t := t;

  let last: t => (option a);
  let lastOrRaise: t => a;
  let reduceReversed: while_::('acc => a => bool)? => ('acc => a => 'acc) => 'acc => t => 'acc;
  let toCollectionReversed: t => (Collection.t a);
  let toIterableReversed: t => (Iterable.t a);
  let toNavigableCollection: t => (navigableCollection a);
  let toNavigableCollectionReversed: t => (navigableCollection a);
  let toSequenceReversed: t => (Sequence.t a);
  let toSequentialCollectionReversed: t => (SequentialCollection.t a);
};

module type S1 = {
  type t 'a;

  include SequentialCollection.S1 with type t 'a := t 'a;

  let last: (t 'a) => (option 'a);
  let lastOrRaise: (t 'a) => 'a;
  let reduceReversed: while_::('acc => 'a => bool)? => ('acc => 'a => 'acc) => 'acc => (t 'a) => 'acc;
  let toCollectionReversed: t 'a => (Collection.t 'a);
  let toIterableReversed: t 'a => (Iterable.t 'a);
  let toNavigableCollection: (t 'a) => (navigableCollection 'a);
  let toNavigableCollectionReversed: (t 'a) => (navigableCollection 'a);
  let toSequenceReversed: (t 'a) => (Sequence.t 'a);
  let toSequentialCollectionReversed: t 'a => (SequentialCollection.t 'a);
};

let module Make = fun (Base: {
  type a;
  type t;

  let count: t => int;
  let firstOrRaise: t => a;
  let lastOrRaise: t => a;
  let reduce: while_::('acc => a => bool) => ('acc => a => 'acc) => 'acc => t => 'acc;
  let reduceReversed: while_::('acc => a => bool) => ('acc => a => 'acc) => 'acc => t => 'acc;
  let toSequence: t => Sequence.t a;
  let toSequenceReversed: t => Sequence.t a;
}) => ({
  include Base;

  include (SequentialCollection.Make Base: SequentialCollection.S with type t := t and type a := a);

  let last (collection: t): (option a) =>
    if (isEmpty collection) None
    else Some (lastOrRaise collection);

  let module ReversedSequentialCollection = SequentialCollection.Make {
    type nonrec t = t;
    type nonrec a = a;

    let count = count;
    let firstOrRaise = lastOrRaise;
    let reduce = Base.reduceReversed;
    let toSequence = toSequenceReversed;
  };

  let reduceReversed = ReversedSequentialCollection.reduce;

  let toIterableReversed = ReversedSequentialCollection.toIterable;

  let toSequenceReversed = ReversedSequentialCollection.toSequence;

  let toCollectionReversed = ReversedSequentialCollection.toCollection;

  let toSequentialCollectionReversed = ReversedSequentialCollection.toSequentialCollection;

  let navigableCollectionBase: SequentialCollection.s t a = {
    count,
    firstOrRaise,
    reduce: Base.reduce,
    toSequence,
  };

  let navigableCollectionReversedBase: SequentialCollection.s t a = {
    count,
    firstOrRaise: lastOrRaise,
    reduce: Base.reduceReversed,
    toSequence: toSequenceReversed,
  };

  let toNavigableCollection (collection: t): (navigableCollection a) =>
    if (isEmpty collection) Empty
    else Instance collection navigableCollectionBase navigableCollectionReversedBase;

  let toNavigableCollectionReversed (collection: t): (navigableCollection a) =>
    if (isEmpty collection) Empty
    else Instance collection navigableCollectionReversedBase navigableCollectionBase;

}: S with type t := Base.t and type a := Base.a);

let module Make1 = fun (Base: {
  type t 'a;

  let count: t 'a => int;
  let firstOrRaise: t 'a => 'a;
  let lastOrRaise: t 'a => 'a;
  let reduce: while_::('acc => 'a => bool) => ('acc => 'a => 'acc) => 'acc => t 'a => 'acc;
  let reduceReversed: while_::('acc => 'a => bool) => ('acc => 'a => 'acc) => 'acc => t 'a => 'acc;
  let toSequence: t 'a => Sequence.t 'a;
  let toSequenceReversed: t 'a => Sequence.t 'a;
}) => ({
  include Base;

  include (SequentialCollection.Make1 Base: SequentialCollection.S1 with type t 'a := t 'a);

  let last (collection: t 'a): (option 'a) =>
    if (isEmpty collection) None
    else Some (lastOrRaise collection);

  let module ReversedSequentialCollection = SequentialCollection.Make1 {
    type nonrec t 'a = t 'a;

    let count = count;
    let firstOrRaise = lastOrRaise;
    let reduce = Base.reduceReversed;
    let toSequence = toSequenceReversed;
  };

  let reduceReversed = ReversedSequentialCollection.reduce;

  let toIterableReversed = ReversedSequentialCollection.toIterable;

  let toSequenceReversed = ReversedSequentialCollection.toSequence;

  let toCollectionReversed = ReversedSequentialCollection.toCollection;

  let toSequentialCollectionReversed = ReversedSequentialCollection.toSequentialCollection;

  let navigableCollectionBase: SequentialCollection.s (t 'a) 'a = {
    count,
    firstOrRaise,
    reduce: Base.reduce,
    toSequence,
  };

  let navigableCollectionReversedBase: SequentialCollection.s (t 'a) 'a = {
    count,
    firstOrRaise: lastOrRaise,
    reduce: Base.reduceReversed,
    toSequence: toSequenceReversed,
  };

  let toNavigableCollection (collection: t 'a): (navigableCollection 'a) =>
    if (isEmpty collection) Empty
    else Instance collection navigableCollectionBase navigableCollectionReversedBase;

  let toNavigableCollectionReversed (collection: t 'a): (navigableCollection 'a) =>
    if (isEmpty collection) Empty
    else Instance collection navigableCollectionReversedBase navigableCollectionBase;

}: S1 with type t 'a := Base.t 'a);

include(Make1 {
  type nonrec t 'a = t 'a;

  let count (collection: t 'a): int => switch collection {
    | Empty => 0
    | Instance collection { count } _ => count collection
  };

  let firstOrRaise (collection: t 'a): 'a => switch collection {
    | Empty => failwith "empty"
    | Instance collection { firstOrRaise } _ => firstOrRaise collection
  };

  let lastOrRaise (collection: t 'a): 'a => switch collection {
    | Empty => failwith "empty"
    | Instance collection _ { firstOrRaise } => firstOrRaise collection
  };

  let reduce
      while_::(predicate: 'acc => 'a => bool)
      (f: 'acc => 'a => 'acc)
      (acc: 'acc)
      (collection: t 'a): 'acc => switch collection {
    | Empty => acc
    | Instance collection { reduce } _ =>
        collection |> reduce while_::predicate f acc;
  };

  let reduceReversed
      while_::(predicate: 'acc => 'a => bool)
      (f: 'acc => 'a => 'acc)
      (acc: 'acc)
      (collection: t 'a): 'acc => switch collection {
    | Empty => acc
    | Instance collection _ { reduce } =>
        collection |> reduce while_::predicate f acc;
  };

  let toSequence (collection: t 'a): (Sequence.t 'a) => switch collection {
    | Empty => Sequence.empty ()
    | Instance collection { toSequence } _ => toSequence collection
  };

  let toSequenceReversed (collection: t 'a): (Sequence.t 'a) => switch collection {
    | Empty => Sequence.empty ()
    | Instance collection _ { toSequence } => toSequence collection
  };
}: S1 with type t 'a := t 'a);

let empty (): (t 'a) => Empty;

let toNavigableCollection (collection: t 'a): (t 'a) =>
  collection;
